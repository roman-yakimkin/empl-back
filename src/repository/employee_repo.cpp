//
// Created by Roman Yakimkin on 02.06.2026.
//

#include "employee_repo.h"
#include "core/errors/data_access_exception.h"
#include "helpers/pqxx.h"
#include "helpers/strings.h"

namespace repo {
    std::string employee_repo::build_raw_query() const {
        return R"(
            select e.id,
                   e.name,
                   e.department_id,
                   d.name as dep_name,
                   e.position_id,
                   p.name as pos_name,
                   EXTRACT(EPOCH FROM created_at)::bigint AS created_at,
                   EXTRACT(EPOCH FROM updated_at)::bigint AS updated_at
            from employees e
            inner join departments d on e.department_id = d.id
            inner join positions p on e.position_id = p.id
        )";
    }

    void employee_repo::build_filtered_query(const core::filters::employee &filter, std::string& query, pqxx::params &params) const {
        query = build_raw_query();

        int idx = 1;
        bool first_cnd = true;

        auto add = [&](const std::string& cnd, auto&& value) {
            query += (first_cnd ? " where " : " and " ) + cnd;
            params.append(std::forward<decltype(value)>(value));

            idx++;
            first_cnd = false;
        };

        if (filter.department_id) add("department_id = $" + std::to_string(idx), filter.department_id.value());
        if (filter.position_id) add("position_id = $" + std::to_string(idx), filter.position_id.value());
        if (filter.name_pattern) {
            const std::string escaped = helpers::escape_like_pattern(filter.name_pattern.value());
            add("e.name ILIKE $" + std::to_string(idx) + " ESCAPE '\\'",
                "%" + escaped + "%");
        }

        query += " order by name";
    }

    core::employee_detail employee_repo::detail_from_db_row(const pqxx::row &row) const {
        core::employee_detail result;

        result.id = row["id"].as<int>();
        result.name = row["name"].as<std::string>();
        result.dep.id = row["department_id"].as<int>();
        result.dep.name = row["dep_name"].as<std::string>();
        result.pos.id = row["position_id"].as<int>();
        result.pos.name = row["pos_name"].as<std::string>();
        result.created_at = row["created_at"].as<int64_t>();
        result.updated_at = row["updated_at"].as<int64_t>();

        return result;
    }

    core::employee employee_repo::from_db_row(const pqxx::row &row) const {
        core::employee result;

        result.id = row["id"].as<int>();
        result.name = row["name"].as<std::string>();
        result.department_id = row["department_id"].as<int>();
        result.position_id = row["position_id"].as<int>();
        result.created_at = row["created_at"].as<int64_t>();
        result.updated_at = row["updated_at"].as<int64_t>();

        return result;
    }

    std::optional<core::employee_detail> employee_repo::get_one(int id) const {
        try {
            pqxx::nontransaction tx(this->db->get_connection());
            auto res = tx.exec(build_raw_query() + " where e.id = $1", pqxx::params(id));
            if (res.empty()) return std::nullopt;

            if (res.size() > 1) {
                logger->critical("Data integrity error: {} rows for id={}", res.size(), id);
                throw core::data_access_exception("Duplicate employee id detected");
            }

            auto result = detail_from_db_row(res[0]);

            logger->debug("Employee fetched successfully: {}", id);

            return result;
        }
        catch (const pqxx::transaction_rollback& e) {
            logger->warn("Transaction rolled back: {}", e.what());
            throw core::data_access_exception("Transaction rolled back", std::current_exception());
        }
        catch (const pqxx::sql_error& e) {
            logger->error("SQL error (id = {}): {} | Query: {}", id, e.what(), e.query());
            throw core::data_access_exception("Failed to execute database query", std::current_exception());
        }
        catch (const pqxx::broken_connection& e) {
            logger->critical("DB connection lost (id = {}): {}", id, e.what());
            throw core::data_access_exception("Database connection is unavailable", std::current_exception());
        }
        catch (const std::exception& e) {
            logger->error("Unexpected DB error (id={}): {}", id, e.what());
            throw core::data_access_exception("Unexpected database failure", std::current_exception());
        }
    }

    std::vector<core::employee_detail> employee_repo::get_all(const core::filters::employee &filter) const {
        std::string query;
        pqxx::params params;

        build_filtered_query(filter, query, params);

        try {
            std::vector<core::employee_detail> items;
            pqxx::nontransaction tx(this->db->get_connection());

            pqxx::result res = tx.exec(query, params);
            items.reserve(res.size());

            for (auto const& row : res) items.emplace_back(detail_from_db_row(row));

            logger->debug("Employee list fetched successfully");
            return items;
        }
        catch (const pqxx::transaction_rollback& e) {
            logger->warn("Transaction rolled back: {}", e.what());
            throw core::data_access_exception("Transaction rolled back", std::current_exception());
        }
        catch (const pqxx::sql_error& e) {
            logger->error("SQL error: {} | Query: {}", e.what(), e.query());
            throw core::data_access_exception("Failed to execute database query", std::current_exception());
        }
        catch (const pqxx::broken_connection& e) {
            logger->critical("DB connection broken: {}", e.what());
            throw core::data_access_exception("Database connection is unavailable", std::current_exception());
        }
        catch (const std::exception& e) {
            logger->error("Unexpected DB error: {}", e.what());
            throw core::data_access_exception("Unexpected database failure", std::current_exception());
        }
    }

    core::employee employee_repo::create(const core::employee &e) {
        try {
            pqxx::work tx(this->db->get_connection());

            auto res = tx.exec(R"(
                INSERT INTO employees (name, department_id, position_id, created_at, updated_at)
                VALUES ($1, $2, $3, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)
                RETURNING id, name, department_id, position_id,
                          EXTRACT(EPOCH FROM created_at)::bigint AS created_at,
                          EXTRACT(EPOCH FROM updated_at)::bigint AS updated_at
            )", pqxx::params(e.name, e.department_id, e.position_id)).one_row();

            auto result = from_db_row(res);

            tx.commit();

            logger->debug("Employee created successfully");

            return result;
        }
        catch (const pqxx::foreign_key_violation& ex) {
            throw helpers::get_validation_exception(ex, logger);
        }
        catch (const pqxx::unexpected_rows& ex) {
            logger->warn("insert returned no rows: {}", ex.what());
            throw core::data_access_exception("Data insertion returned no ID", std::current_exception());
        }
        catch (const pqxx::transaction_rollback& ex) {
            logger->warn("Transaction rolled back: {}", ex.what());
            throw core::data_access_exception("Transaction rolled back", std::current_exception());
        }
        catch (const pqxx::sql_error& ex) {
            logger->error("SQL error: {} | Query: {}", ex.what(), ex.query());
            throw core::data_access_exception("Failed to execute database query", std::current_exception());
        }
        catch (const pqxx::broken_connection& ex) {
            logger->critical("DB connection broken: {}", ex.what());
            throw core::data_access_exception("Database connection is unavailable", std::current_exception());
        }
        catch (const std::exception& ex) {
            logger->error("Unexpected DB error: {}", ex.what());
            throw core::data_access_exception("Unexpected database failure", std::current_exception());
        }
    }

    std::optional<core::employee> employee_repo::update(int id, const core::employee &e) {
        try {
            pqxx::work tx(this->db->get_connection());

            auto res = tx.exec(R"(
                UPDATE employees
                SET name = $1, department_id = $2, position_id = $3, updated_at = CURRENT_TIMESTAMP
                WHERE id = $4
                RETURNING id, name, department_id, position_id,
                          EXTRACT(EPOCH FROM created_at)::bigint AS created_at,
                          EXTRACT(EPOCH FROM updated_at)::bigint AS updated_at
            )", pqxx::params(e.name, e.department_id, e.position_id, id));

            if (!res.affected_rows()) {
                logger->debug("No employees updated");
                tx.abort();

                return std::nullopt;
            }


            auto result = from_db_row(res[0]);

            tx.commit();

            logger->debug("Employee updated successfully");

            return result;
        }
        catch (const pqxx::foreign_key_violation& ex) {
            throw helpers::get_validation_exception(ex, logger);
        }
        catch (const pqxx::unexpected_rows& ex) {
            logger->warn("insert returned no rows: {}", ex.what());
            throw core::data_access_exception("Data update returned no ID", std::current_exception());
        }
        catch (const pqxx::transaction_rollback& ex) {
            logger->warn("Transaction rolled back: {}", ex.what());
            throw core::data_access_exception("Transaction rolled back", std::current_exception());
        }
        catch (const pqxx::sql_error& ex) {
            logger->error("SQL error: {} | Query: {}", ex.what(), ex.query());
            throw core::data_access_exception("Failed to execute database query", std::current_exception());
        }
        catch (const pqxx::broken_connection& ex) {
            logger->critical("DB connection broken: {}", ex.what());
            throw core::data_access_exception("Database connection is unavailable", std::current_exception());
        }
        catch (const std::exception& ex) {
            logger->error("Unexpected DB error: {}", ex.what());
            throw core::data_access_exception("Unexpected database failure", std::current_exception());
        }
    }

    bool employee_repo::del(int id) {
        try {
            pqxx::work tx(this->db->get_connection());

            auto res = tx.exec("DELETE FROM employees WHERE id=$1", pqxx::params(id));

            if (!res.affected_rows()) {
                tx.abort();
                logger->debug("No employees deleted");
                return false;
            }

            tx.commit();
            logger->debug("Employee deleted successfully");

            return true;
        }
        catch (const pqxx::transaction_rollback& e) {
            logger->warn("Transaction rolled back: {}", e.what());
            throw core::data_access_exception("Transaction rolled back", std::current_exception());
        }
        catch (const pqxx::sql_error& e) {
            logger->error("SQL error (id = {}): {} | Query: {}", id, e.what(), e.query());
            throw core::data_access_exception("Failed to execute database query", std::current_exception());
        }
        catch (const pqxx::broken_connection& e) {
            logger->critical("DB connection lost (id = {}): {}", id, e.what());
            throw core::data_access_exception("Database connection is unavailable", std::current_exception());
        }
        catch (const std::exception& e) {
            logger->error("Unexpected DB error (id={}): {}", id, e.what());
            throw core::data_access_exception("Unexpected database failure", std::current_exception());
        }
    }

} // repo
//
// Created by Roman Yakimkin on 31.05.2026.
//

#include "position_repo.h"

#include "core/errors/data_access_exception.h"
#include "core/errors/duplicate_entry_exception.h"
#include "helpers/pqxx.h"

namespace repo {
    std::optional<core::position> position_repo::get_one(int id) const {
        try {
            pqxx::nontransaction tx(this->db->get_connection());
            auto res = tx.exec("SELECT id, name FROM positions WHERE id = $1",pqxx::params(id));
            if (res.empty()) return std::nullopt;

            if (res.size() > 1) {
                logger->critical("Data integrity error: {} rows for id={}", res.size(), id);
                throw core::data_access_exception("Duplicate position id detected");
            }

            auto _id = res[0][0].as<int>();
            auto _name = res[0][1].as<std::string>();

            logger->debug("position fetched successfully: {}", id);

            return core::position(_id, _name);
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

    std::map<int, core::position> position_repo::get_all() const {
        try {
            std::map<int, core::position> items;
            pqxx::nontransaction tx(this->db->get_connection());

            for (const auto& [id, name] : tx.query<int, std::string>(
                "SELECT id, name FROM positions")) {
                items.emplace(id, core::position{id, name});
            }

            logger->debug("position list fetched successfully");
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

    core::position position_repo::create(const core::position& p) {
        try {
            pqxx::work tx(this->db->get_connection());

            auto res = tx.exec("INSERT INTO positions (name) VALUES ($1) RETURNING id", pqxx::params(p.name)).one_field();
            const int id = res.as<int>();

            tx.commit();

            logger->debug("Position created successfully");

            return core::position(id, p.name);
        }
        catch (const pqxx::unique_violation& e) {
            logger->warn("Unique constraint violation for department name '{}': {}", p.name, e.what());
            throw core::duplicate_entry_exception("Unique constraint violation for department name", std::current_exception());
        }
        catch (const pqxx::unexpected_rows& e) {
            logger->warn("insert returned no rows: {}", e.what());
            throw core::data_access_exception("Data insertion returned no ID", std::current_exception());
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

    std::optional<core::position> position_repo::update(int id, const core::position &p) {
        try {
            pqxx::work tx(this->db->get_connection());

            auto res = tx.exec("UPDATE positions set name = $1 WHERE id = $2", pqxx::params(p.name, id));

            if (!res.affected_rows()) {
                logger->debug("No positions updated");
                tx.abort();

                return std::nullopt;
            }

            tx.commit();
            logger->debug("position updated successfully");

            return core::position(id, p.name);
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

    bool position_repo::del(int id) {
        try {
            pqxx::work tx(this->db->get_connection());

            auto res = tx.exec("DELETE FROM positions WHERE id=$1", pqxx::params(id));

            if (!res.affected_rows()) {
                tx.abort();
                logger->debug("No positions deleted");
                return false;
            }

            tx.commit();
            logger->debug("Position deleted successfully");

            return true;
        }
        catch (const pqxx::foreign_key_violation& e) {
            throw helpers::get_validation_exception(e, logger);
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
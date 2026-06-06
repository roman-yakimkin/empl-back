-- migrate:up
create table departments (
    id serial primary key,
    name varchar not null unique,
    CHECK (name <> '')
);

insert into departments (name) values
    ('Управление'),
    ('Бухгалтерия'),
    ('Отдел АСУ'),
    ('Столярный цех'),
    ('Слесарный цех'),
    ('Отдел кадров');

-- migrate:down
drop table departments;


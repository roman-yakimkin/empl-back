-- migrate:up

create table employees (
    id serial primary key,
    name varchar not null,
    department_id int,
    position_id int,
    created_at timestamp default NOW(),
    updated_at timestamp default NOW(),

    constraint fk_department foreign key (department_id) references departments(id) on update cascade ,
    constraint fk_position foreign key (position_id) references positions(id) on update cascade,
    check (name <> '')
);

insert into employees (name, department_id, position_id) values
    ('Иванов Петр Сергеевич', 1, 1),
    ('Петрова Мария Ивановна', 2, 2),
    ('Стрельников Владимир Сергеевич', 3, 3),
    ('Ковалёв Александр Викторович', 5, 5);

-- migrate:down

drop table employees;

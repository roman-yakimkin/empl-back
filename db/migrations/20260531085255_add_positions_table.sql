-- migrate:up

create table positions (
     id serial primary key,
     name varchar not null unique,
     CHECK (name <> '')
);

insert into positions (name) values
    ('Директор'),
    ('Бухгалтер'),
    ('Инженер'),
    ('Водитель'),
    ('Токарь'),
    ('Охранник');

-- migrate:down
drop table positions;


create table if not exists devices (
  id text primary key,
  name text not null,
  type text not null,
  created_at timestamptz not null default now()
);

create table if not exists device_states (
  device_id text primary key references devices(id) on delete cascade,
  desired_state boolean not null default false,
  reported_state boolean not null default false,
  updated_at timestamptz not null default now(),
  reported_at timestamptz
);

create table if not exists control_logs (
  id bigserial primary key,
  device_id text not null references devices(id) on delete cascade,
  requested_state boolean not null,
  source text not null,
  result text not null,
  created_at timestamptz not null default now()
);

create table if not exists telemetry_readings (
  id bigserial primary key,
  temperature numeric(5, 2) not null,
  humidity numeric(5, 2) not null,
  smoke_ppm integer not null,
  measured_at timestamptz not null default now()
);

insert into devices (id, name, type)
values
  ('led', 'LED Light', 'relay'),
  ('servo', 'Servo Motor', 'servo'),
  ('buzzer', 'Buzzer Alarm', 'buzzer'),
  ('pump', 'Mini Water Pump', 'pump')
on conflict (id) do nothing;

insert into device_states (device_id, desired_state, reported_state)
values
  ('led', false, false),
  ('servo', false, false),
  ('buzzer', false, false),
  ('pump', false, false)
on conflict (device_id) do nothing;

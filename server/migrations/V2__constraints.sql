-- Add FK, index and status constraint

ALTER TABLE tasks
  ADD CONSTRAINT fk_tasks_user
  FOREIGN KEY (user_id) REFERENCES users(id)
  ON DELETE CASCADE;

CREATE INDEX IF NOT EXISTS idx_tasks_user_id ON tasks(user_id);

ALTER TABLE tasks
  ADD CONSTRAINT tasks_status_check
  CHECK (status IN ('InProject','InProgress','Done'));


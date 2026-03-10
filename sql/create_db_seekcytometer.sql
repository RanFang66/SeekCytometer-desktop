DROP DATABASE IF EXISTS "SeekCytometer";

CREATE DATABASE "SeekCytometer";

\c  "SeekCytometer";

\i trigger_functions.sql

CREATE TABLE Users (
    user_id          SERIAL PRIMARY KEY NOT NULL,
    user_name        VARCHAR(32) NOT NULL UNIQUE,
    user_admin       BOOLEAN NOT NULL,
    department       VARCHAR(32),
    email            VARCHAR(64),
    user_password    TEXT NOT NULL DEFAULT '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92' -- '123456'
);


CREATE TABLE Experiments (
    experiment_id   SERIAL PRIMARY KEY NOT NULL,
    user_id         INT NOT NULL, 
    experiment_name VARCHAR(64) NOT NULL,
    experiment_path TEXT,
    FOREIGN KEY (user_id) REFERENCES Users(user_id) ON DELETE CASCADE,
    UNIQUE (user_id, experiment_name)
);

CREATE TABLE Specimens (
    specimen_id         SERIAL PRIMARY KEY NOT NULL,
    experiment_id       INT NOT NULL,
    specimen_name       VARCHAR(64) NOT NULL,
    FOREIGN KEY (experiment_id) REFERENCES Experiments(experiment_id) ON DELETE CASCADE,
    UNIQUE (experiment_id, specimen_name)
);

CREATE TABLE Tubes (
    tube_id             SERIAL PRIMARY KEY NOT NULL,
    specimen_id         INT NOT NULL,
    tube_name           VARCHAR(64) NOT NULL,

    FOREIGN KEY (specimen_id) REFERENCES Specimens(specimen_id) ON DELETE CASCADE,
    UNIQUE (specimen_id, tube_name)
);

CREATE INDEX idx_experiments_user_id ON Experiments(user_id);
CREATE INDEX idx_specimens_experiment_id ON Specimens(experiment_id);
CREATE INDEX idx_tubes_specimen_id ON Tubes(specimen_id);


CREATE TYPE ParentType AS ENUM ('Experiment', 'Specimen', 'Tube');
CREATE TYPE ThresholdOperator AS ENUM ('OR', 'AND');

CREATE TABLE CytometerSettings (
    setting_id          SERIAL PRIMARY KEY NOT NULL,
    setting_name        VARCHAR(64) NOT NULL DEFAULT 'CytometerSettings',
    parent_type         ParentType NOT NULL,
    experiment_id       INT REFERENCES Experiments(experiment_id) ON DELETE CASCADE,
    specimen_id         INT REFERENCES Specimens(specimen_id) ON DELETE CASCADE,
    tube_id             INT REFERENCES Tubes(tube_id) ON DELETE CASCADE,
    threshold_op        ThresholdOperator NOT NULL DEFAULT 'OR',

    CHECK (
        (parent_type = 'Experiment' AND experiment_id IS NOT NULL AND specimen_id IS NULL AND tube_id IS NULL) OR
        (parent_type = 'Specimen' AND specimen_id IS NOT NULL AND experiment_id IS NULL AND tube_id IS NULL) OR
        (parent_type = 'Tube' AND tube_id IS NOT NULL AND experiment_id IS NULL AND specimen_id IS NULL)
    ),

    UNIQUE (parent_type, experiment_id),
    UNIQUE (parent_type, specimen_id),
    UNIQUE (parent_type, tube_id)
);



-- CREATE TABLE CytometerSettings (
--     setting_id          SERIAL PRIMARY KEY NOT NULL,
--     setting_name        VARCHAR(64) NOT NULL DEFAULT 'CytometerSettings',
--     parent_type         ParentType NOT NULL,
--     parent_id           INT NOT NULL,
--     threshold_op        ThresholdOperator NOT NULL DEFAULT 'OR',

--     CONSTRAINT fk_parent_experiment FOREIGN KEY (parent_id) REFERENCES Experiments(experiment_id) ON DELETE CASCADE,
--     CONSTRAINT fk_parent_specimen FOREIGN KEY (parent_id) REFERENCES Specimens(specimen_id) ON DELETE CASCADE,
--     CONSTRAINT fk_parent_tube FOREIGN KEY (parent_id) REFERENCES Tubes(tube_id) ON DELETE CASCADE,
--     UNIQUE (parent_type, parent_id)
-- );

-- CREATE INDEX idx_settings_parent ON CytometerSettings(parent_type, parent_id);

CREATE TYPE NodeType AS ENUM ('Root', 'User', 'Experiment', 'Specimen', 'Tube', 'Settings', 'Worksheet');
CREATE TABLE BrowserData (
    id SERIAL PRIMARY KEY NOT NULL,
    parent_id INT,
    node_name VARCHAR(64) NOT NULL,
    node_type NodeType NOT NULL,
    node_id INT NOT NULL,
    depth INT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at  TIMESTAMP DEFAULT CURRENT_TIMESTAMP                            
);

CREATE TABLE Detectors (
    detector_id         SERIAL PRIMARY KEY NOT NULL,
    detector_name       VARCHAR(64) NOT NULL,
    detector_type       VARCHAR(64) NOT NULL,
    filter_peak         INT NOT NULL,
    filter_bandwidth    INT NOT NULL,
    default_gain        INT NOT NULL DEFAULT 100,
    default_offset      INT NOT NULL DEFAULT 0
);

CREATE TABLE DetectorSettings (
    detector_setting_id SERIAL PRIMARY KEY NOT NULL,
    setting_id       INT NOT NULL,
    detector_id      INT NOT NULL,
    parameter_name   VARCHAR(64) NOT NULL,
    detector_gain    INT NOT NULL DEFAULT 100,
    detector_offset  INT NOT NULL DEFAULT 0,
    enable_threshold BOOLEAN NOT NULL DEFAULT FALSE,
    threshold_value  INT NOT NULL DEFAULT 1000,
    enable_height    BOOLEAN NOT NULL DEFAULT FALSE,
    enable_width     BOOLEAN NOT NULL DEFAULT FALSE,
    enable_area      BOOLEAN NOT NULL DEFAULT FALSE,
    FOREIGN KEY (setting_id) REFERENCES CytometerSettings(setting_id) ON DELETE CASCADE,
    FOREIGN KEY (detector_id) REFERENCES Detectors(detector_id) ON DELETE CASCADE,
    UNIQUE (setting_id, parameter_name),
    UNIQUE (setting_id, detector_id)
);



CREATE TABLE WorkSheets (
    worksheet_id        SERIAL PRIMARY KEY NOT NULL,
    is_global           BOOLEAN NOT NULL DEFAULT TRUE,
    experiment_id       INT,
    tube_id             INT,
    worksheet_name      VARCHAR(64) NOT NULL,   

    CHECK (
        (is_global = TRUE AND experiment_id IS NOT NULL AND tube_id IS NULL) OR
        (is_global = FALSE AND experiment_id IS NULL AND tube_id IS NOT NULL)
    ),
    UNIQUE (experiment_id, worksheet_name),
    UNIQUE (tube_id, worksheet_name)
);


CREATE TYPE PlotType AS ENUM('Histogram', 'Scatter', 'Contour');
CREATE TYPE MeasureType AS ENUM('Height', 'Width', 'Area');
CREATE TABLE Plots (
    plot_id             SERIAL PRIMARY KEY NOT NULL,
    worksheet_id        INT NOT NULL,
    plot_type           PlotType NOT NULL,
    plot_name           VARCHAR(64) NOT NULL,
    x_axis_id           INT NOT NULL,
    y_axis_id           INT,
    x_measure_type      MeasureType NOT NULL DEFAULT 'Height',
    y_measure_type      MeasureType, 
    plot_size           INT NOT NULL DEFAULT 512, 
    FOREIGN KEY (worksheet_id) REFERENCES WorkSheets(worksheet_id) ON DELETE CASCADE,
    FOREIGN KEY (x_axis_id) REFERENCES DetectorSettings(detector_setting_id) ON DELETE CASCADE,
    FOREIGN KEY (y_axis_id) REFERENCES DetectorSettings(detector_setting_id) ON DELETE CASCADE,
    UNIQUE (worksheet_id, plot_name)
);


-- CREATE TYPE PopulationLogicOperator AS ENUM('NONE', 'AND', 'OR', 'NOT', 'REST');
-- CREATE TABLE Populations (
--     population_id       SERIAL PRIMARY KEY NOT NULL,
--     worksheet_id        INT NOT NULL,
--     population_name     TEXT NOT NULL,
--     parent_id           INT,
--     logic_op            PopulationLogicOperator NOT NULL DEFAULT 'NONE',
--     FOREIGN KEY (worksheet_id) REFERENCES WorkSheets(worksheet_id) ON DELETE CASCADE,
--     FOREIGN KEY (parent_id) REFERENCES Populations(population_id) ON DELETE CASCADE
-- );



CREATE TYPE GateType AS ENUM('rectangle', 'polygon', 'ellipse', 'interval', 'quadrant');
CREATE TABLE Gates (
    gate_id                     SERIAL PRIMARY KEY NOT NULL,
    worksheet_id                INT NOT NULL,
    gate_name                   VARCHAR(64) NOT NULL,
    gate_type                   GateType NOT NULL,
    parent_population_id        INT,
    x_axis_id                   INT NOT NULL,
    y_axis_id                   INT,
    x_mearsure_type             MeasureType NOT NULL DEFAULT 'Height',
    y_mearsure_type             MeasureType,
    gate_data                   JSONB NOT NULL,
    FOREIGN KEY (worksheet_id) REFERENCES WorkSheets(worksheet_id) ON DELETE CASCADE,
    FOREIGN KEY (x_axis_id) REFERENCES DetectorSettings(detector_setting_id) ON DELETE CASCADE,
    FOREIGN KEY (y_axis_id) REFERENCES DetectorSettings(detector_setting_id) ON DELETE CASCADE,
    UNIQUE (worksheet_id, gate_name)
);



-- CREATE TABLE PopulationRelation (
--     relation_id                     SERIAL PRIMARY KEY NOT NULL,
--     population_id                   INT NOT NULL,
--     compose_population_id           INT,
--     gate_id                         INT,
--     FOREIGN KEY (population_id) REFERENCES Populations(population_id) ON DELETE CASCADE,
--     FOREIGN KEY (compose_population_id) REFERENCES Populations(population_id) ON DELETE CASCADE,
--     FOREIGN KEY (gate_id) REFERENCES Gates(gate_id) ON DELETE CASCADE,
--     UNIQUE (population_id, compose_population_id),
--     UNIQUE (relation_id, gate_id)
-- );



CREATE TRIGGER AfterUserInsert
AFTER INSERT ON Users
FOR EACH ROW
EXECUTE FUNCTION after_user_insert_function();

CREATE TRIGGER AfterUsersUpdate
AFTER UPDATE ON Users
FOR EACH ROW
EXECUTE FUNCTION after_user_update_function();

CREATE TRIGGER AfterUsersDelete
AFTER DELETE ON Users
FOR EACH ROW
EXECUTE FUNCTION after_user_delete_function();

CREATE TRIGGER AfterExperimentInsert
AFTER INSERT ON Experiments
FOR EACH ROW
EXECUTE FUNCTION after_experiment_insert_function();

CREATE TRIGGER AfterExperimentsUpdate
AFTER UPDATE ON Experiments
FOR EACH ROW
EXECUTE FUNCTION after_experiment_update_function();

CREATE TRIGGER AfterExperimentsDelete
AFTER DELETE ON Experiments
FOR EACH ROW
EXECUTE FUNCTION after_experiment_delete_function();


CREATE TRIGGER AfterSpecimensInsert
AFTER INSERT ON Specimens
FOR EACH ROW
EXECUTE FUNCTION after_specimen_insert_function();

CREATE TRIGGER AfterSpecimensUpdate
AFTER UPDATE ON Specimens
FOR EACH ROW
EXECUTE FUNCTION after_specimen_update_function();

CREATE TRIGGER AfterSpecimensDelete
AFTER DELETE ON Specimens
FOR EACH ROW
EXECUTE FUNCTION after_specimen_delete_function();

CREATE TRIGGER AfterTubesInsert
AFTER INSERT ON Tubes
FOR EACH ROW
EXECUTE FUNCTION after_tube_insert_function();

CREATE TRIGGER AfterTubesUpdate
AFTER UPDATE ON Tubes
FOR EACH ROW
EXECUTE FUNCTION after_tube_update_function();

CREATE TRIGGER AfterTubesDelete
AFTER DELETE ON Tubes
FOR EACH ROW
EXECUTE FUNCTION after_Tube_delete_function();


CREATE TRIGGER AfterCytometerSettingsInsert
AFTER INSERT ON CytometerSettings
FOR EACH ROW
EXECUTE FUNCTION after_settings_insert_function();

CREATE TRIGGER AfterCytometerSettingsUpdate
AFTER UPDATE ON CytometerSettings
FOR EACH ROW
EXECUTE FUNCTION after_settings_update_function();

CREATE TRIGGER AfterCytometerSettingsDelete
AFTER DELETE ON CytometerSettings
FOR EACH ROW
EXECUTE FUNCTION after_settings_delete_function();

CREATE TRIGGER AfterWorksheetsInsert
AFTER INSERT ON WorkSheets
FOR EACH ROW
EXECUTE FUNCTION after_worksheet_insert_function();

CREATE TRIGGER AfterWorksheetsUpdate
AFTER UPDATE ON WorkSheets
FOR EACH ROW
EXECUTE FUNCTION after_worksheet_update_function();

CREATE TRIGGER AfterWorksheetsDelete
AFTER DELETE ON WorkSheets
FOR EACH ROW
EXECUTE FUNCTION after_worksheet_delete_function();


-- CREATE TRIGGER AfterGatesInsert
-- AFTER INSERT ON Gates
-- FOR EACH ROW
-- EXECUTE FUNCTION after_gate_insert_function();

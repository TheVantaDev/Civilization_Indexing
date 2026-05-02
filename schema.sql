-- ============================================================
--  AI Civilisation Ethics Indexer  –  MySQL Schema
--  Run with: mysql -u root -pSam@2006 < schema.sql
-- ============================================================

CREATE DATABASE IF NOT EXISTS ai_civilisation
    CHARACTER SET utf8mb4
    COLLATE utf8mb4_unicode_ci;

USE ai_civilisation;

-- ----------------------------------------------------------
-- 1. AI Systems master table
-- ----------------------------------------------------------
CREATE TABLE IF NOT EXISTS ai_systems (
    ai_system_id      INT          NOT NULL,
    system_name       VARCHAR(128) NOT NULL,
    application_domain VARCHAR(128) NOT NULL,
    developer         VARCHAR(128) NOT NULL,
    deployment_region VARCHAR(64)  NOT NULL,
    PRIMARY KEY (ai_system_id)
) ENGINE=InnoDB;

-- ----------------------------------------------------------
-- 2. Behaviors  (real-time Ollama rows land here)
-- ----------------------------------------------------------
CREATE TABLE IF NOT EXISTS ai_behavior (
    record_id      INT          NOT NULL AUTO_INCREMENT,
    ai_system_id   INT          NOT NULL,
    decision_type  VARCHAR(128) NOT NULL,
    affected_group VARCHAR(128) NOT NULL,
    outcome        VARCHAR(128) NOT NULL,
    transparency   ENUM('Low','Medium','High') NOT NULL,
    source         ENUM('csv','ollama','ollama_scenario') NOT NULL DEFAULT 'csv',
    created_at     TIMESTAMP    NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (record_id),
    INDEX idx_ai_id (ai_system_id)
) ENGINE=InnoDB;

-- ----------------------------------------------------------
-- 3. Intermediate scoring  (per AI × principle)
-- ----------------------------------------------------------
CREATE TABLE IF NOT EXISTS intermediate_scoring (
    id               INT          NOT NULL AUTO_INCREMENT,
    run_id           TIMESTAMP    NOT NULL DEFAULT CURRENT_TIMESTAMP,
    ai_system_id     INT          NOT NULL,
    system_name      VARCHAR(128) NOT NULL,
    ethical_principle VARCHAR(128) NOT NULL,
    alignment_score  FLOAT        NOT NULL,
    PRIMARY KEY (id),
    INDEX idx_ai_id (ai_system_id)
) ENGINE=InnoDB;

-- ----------------------------------------------------------
-- 4. Cultural compatibility  (per AI × region)
-- ----------------------------------------------------------
CREATE TABLE IF NOT EXISTS cultural_compatibility (
    id                  INT          NOT NULL AUTO_INCREMENT,
    run_id              TIMESTAMP    NOT NULL DEFAULT CURRENT_TIMESTAMP,
    ai_system_id        INT          NOT NULL,
    system_name         VARCHAR(128) NOT NULL,
    region              VARCHAR(64)  NOT NULL,
    compatibility_score FLOAT        NOT NULL,
    PRIMARY KEY (id),
    INDEX idx_ai_id (ai_system_id)
) ENGINE=InnoDB;

-- ----------------------------------------------------------
-- 5. Risk detection  (per AI × risk category)
-- ----------------------------------------------------------
CREATE TABLE IF NOT EXISTS risk_detection (
    id            INT         NOT NULL AUTO_INCREMENT,
    run_id        TIMESTAMP   NOT NULL DEFAULT CURRENT_TIMESTAMP,
    ai_system_id  INT         NOT NULL,
    system_name   VARCHAR(128) NOT NULL,
    risk_category VARCHAR(64) NOT NULL,
    risk_level    ENUM('Low','Medium','High') NOT NULL,
    risk_score    FLOAT       NOT NULL,
    PRIMARY KEY (id),
    INDEX idx_ai_id (ai_system_id)
) ENGINE=InnoDB;

-- ----------------------------------------------------------
-- 6. Final Ethical AI Civilisation Index
-- ----------------------------------------------------------
CREATE TABLE IF NOT EXISTS final_index (
    id                    INT          NOT NULL AUTO_INCREMENT,
    run_id                TIMESTAMP    NOT NULL DEFAULT CURRENT_TIMESTAMP,
    system_name           VARCHAR(128) NOT NULL,
    ethical_alignment     FLOAT        NOT NULL,
    cultural_compatibility FLOAT       NOT NULL,
    risk_level            ENUM('Low','Medium','High') NOT NULL,
    overall_score         FLOAT        NOT NULL,
    PRIMARY KEY (id)
) ENGINE=InnoDB;

-- ----------------------------------------------------------
-- 7. Dashboard summary  (top-level KPIs per run)
-- ----------------------------------------------------------
CREATE TABLE IF NOT EXISTS dashboard_summary (
    id                      INT          NOT NULL AUTO_INCREMENT,
    run_id                  TIMESTAMP    NOT NULL DEFAULT CURRENT_TIMESTAMP,
    system_name             VARCHAR(128) NOT NULL,
    ethical_alignment       FLOAT        NOT NULL,
    cultural_compatibility  FLOAT        NOT NULL,
    risk_level              ENUM('Low','Medium','High') NOT NULL,
    global_ethics_rank      INT          NOT NULL,
    projected_ethical_score FLOAT        NOT NULL,
    PRIMARY KEY (id)
) ENGINE=InnoDB;

-- Quick sanity check
SELECT 'Schema created successfully.' AS status;

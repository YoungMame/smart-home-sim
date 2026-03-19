#include "db/sqlite_store.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

#include <sqlite3.h>

namespace {

class SqliteConnection {
public:
    explicit SqliteConnection(const std::string& db_path) {
        if (sqlite3_open(db_path.c_str(), &db_) != SQLITE_OK) {
            const std::string message = db_ ? sqlite3_errmsg(db_) : "unknown sqlite open error";
            if (db_) {
                sqlite3_close(db_);
            }
            throw std::runtime_error("[SqliteStore] Failed to open DB: " + message);
        }

        sqlite3_exec(db_, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
    }

    ~SqliteConnection() {
        if (db_) {
            sqlite3_close(db_);
        }
    }

    sqlite3* get() const { return db_; }

private:
    sqlite3* db_{nullptr};
};

void execute_sql(sqlite3* db, const std::string& sql) {
    char* err = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        const std::string message = err ? err : "unknown sqlite error";
        sqlite3_free(err);
        throw std::runtime_error("[SqliteStore] SQL execution failed: " + message);
    }
}

std::string read_text_file(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        throw std::runtime_error("[SqliteStore] Cannot open seed file: " + path);
    }

    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

int table_count(sqlite3* db, const char* table) {
    sqlite3_stmt* stmt = nullptr;
    const char* query = "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name=?;";
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("[SqliteStore] Failed to prepare sqlite_master count query");
    }

    sqlite3_bind_text(stmt, 1, table, -1, SQLITE_TRANSIENT);

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}

int row_count(sqlite3* db, const char* table) {
    const std::string query = std::string("SELECT COUNT(*) FROM ") + table + ";";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("[SqliteStore] Failed to prepare row count query for table " + std::string(table));
    }

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}

} // namespace

void SqliteStore::initialize(const std::string& db_path, const std::string& seed_path) {
    SqliteConnection conn(db_path);
    sqlite3* db = conn.get();

    // Seed file is idempotent (CREATE TABLE IF NOT EXISTS + INSERT OR IGNORE).
    // It is always executed to guarantee schema and default data exist.
    const std::string seed_sql = read_text_file(seed_path);
    execute_sql(db, seed_sql);

    if (table_count(db, "device_models") == 0 || table_count(db, "devices") == 0) {
        throw std::runtime_error("[SqliteStore] Seed failed to create required tables");
    }

    const int models = row_count(db, "device_models");
    if (models == 0) {
        throw std::runtime_error("[SqliteStore] No device model found after seeding");
    }
}

std::vector<VirtualDeviceModel> SqliteStore::load_models(const std::string& db_path) {
    SqliteConnection conn(db_path);
    sqlite3* db = conn.get();

    std::vector<VirtualDeviceModel> models;

    sqlite3_stmt* models_stmt = nullptr;
    const char* models_query = "SELECT id, label, type, protocol FROM device_models ORDER BY id;";
    if (sqlite3_prepare_v2(db, models_query, -1, &models_stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("[SqliteStore] Failed to query device models");
    }

    sqlite3_stmt* caps_stmt = nullptr;
    const char* caps_query = "SELECT capability FROM model_capabilities WHERE model_id = ? ORDER BY capability;";
    if (sqlite3_prepare_v2(db, caps_query, -1, &caps_stmt, nullptr) != SQLITE_OK) {
        sqlite3_finalize(models_stmt);
        throw std::runtime_error("[SqliteStore] Failed to query model capabilities");
    }

    sqlite3_stmt* events_stmt = nullptr;
    const char* events_query = "SELECT event_type FROM model_available_events WHERE model_id = ? ORDER BY event_type;";
    if (sqlite3_prepare_v2(db, events_query, -1, &events_stmt, nullptr) != SQLITE_OK) {
        sqlite3_finalize(caps_stmt);
        sqlite3_finalize(models_stmt);
        throw std::runtime_error("[SqliteStore] Failed to query model available events");
    }

    sqlite3_stmt* aliases_stmt = nullptr;
    const char* aliases_query = "SELECT capability, alias FROM model_capabilities_alias WHERE model_id = ? ORDER BY capability, alias;";
    if (sqlite3_prepare_v2(db, aliases_query, -1, &aliases_stmt, nullptr) != SQLITE_OK) {
        sqlite3_finalize(events_stmt);
        sqlite3_finalize(caps_stmt);
        sqlite3_finalize(models_stmt);
        throw std::runtime_error("[SqliteStore] Failed to query model capability aliases");
    }

    while (sqlite3_step(models_stmt) == SQLITE_ROW) {
        VirtualDeviceModel model;
        model.id = reinterpret_cast<const char*>(sqlite3_column_text(models_stmt, 0));
        model.label = reinterpret_cast<const char*>(sqlite3_column_text(models_stmt, 1));
        model.type = reinterpret_cast<const char*>(sqlite3_column_text(models_stmt, 2));
        model.protocol = reinterpret_cast<const char*>(sqlite3_column_text(models_stmt, 3));

        sqlite3_reset(caps_stmt);
        sqlite3_clear_bindings(caps_stmt);
        sqlite3_bind_text(caps_stmt, 1, model.id.c_str(), -1, SQLITE_TRANSIENT);

        while (sqlite3_step(caps_stmt) == SQLITE_ROW) {
            model.capabilities.emplace_back(
                reinterpret_cast<const char*>(sqlite3_column_text(caps_stmt, 0))
            );
        }

        sqlite3_reset(events_stmt);
        sqlite3_clear_bindings(events_stmt);
        sqlite3_bind_text(events_stmt, 1, model.id.c_str(), -1, SQLITE_TRANSIENT);

        while (sqlite3_step(events_stmt) == SQLITE_ROW) {
            model.available_events.emplace_back(
                reinterpret_cast<const char*>(sqlite3_column_text(events_stmt, 0))
            );
        }

        sqlite3_reset(aliases_stmt);
        sqlite3_clear_bindings(aliases_stmt);
        sqlite3_bind_text(aliases_stmt, 1, model.id.c_str(), -1, SQLITE_TRANSIENT);

        while (sqlite3_step(aliases_stmt) == SQLITE_ROW) {
            const std::string capability = reinterpret_cast<const char*>(sqlite3_column_text(aliases_stmt, 0));
            const std::string alias = reinterpret_cast<const char*>(sqlite3_column_text(aliases_stmt, 1));
            model.capability_aliases[alias] = capability;
        }

        models.push_back(std::move(model));
    }

    sqlite3_finalize(aliases_stmt);
    sqlite3_finalize(events_stmt);
    sqlite3_finalize(caps_stmt);
    sqlite3_finalize(models_stmt);

    return models;
}

std::vector<DeviceRow> SqliteStore::load_devices(const std::string& db_path) {
    SqliteConnection conn(db_path);
    sqlite3* db = conn.get();

    std::vector<DeviceRow> devices;

    sqlite3_stmt* stmt = nullptr;
    const char* query = "SELECT id, label, room, model_id FROM devices ORDER BY id;";
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("[SqliteStore] Failed to query devices");
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        DeviceRow row;
        row.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        row.label = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        row.room = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        row.model_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        devices.push_back(std::move(row));
    }

    sqlite3_finalize(stmt);
    return devices;
}

bool SqliteStore::insert_device(const std::string& db_path,
                                const std::string& id,
                                const std::string& label,
                                const std::string& room,
                                const std::string& model_id) {
    SqliteConnection conn(db_path);
    sqlite3* db = conn.get();

    sqlite3_stmt* stmt = nullptr;
    const char* query = "INSERT INTO devices (id, label, room, model_id) VALUES (?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("[SqliteStore] Failed to prepare insert device statement");
    }

    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, label.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, room.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, model_id.c_str(), -1, SQLITE_TRANSIENT);

    const int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_CONSTRAINT || rc == SQLITE_CONSTRAINT_FOREIGNKEY || rc == SQLITE_CONSTRAINT_PRIMARYKEY) {
        return false;
    }

    if (rc != SQLITE_DONE) {
        throw std::runtime_error("[SqliteStore] Failed to insert device");
    }

    return true;
}

bool SqliteStore::delete_device(const std::string& db_path, const std::string& id) {
    SqliteConnection conn(db_path);
    sqlite3* db = conn.get();

    sqlite3_stmt* stmt = nullptr;
    const char* query = "DELETE FROM devices WHERE id = ?;";
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("[SqliteStore] Failed to prepare delete device statement");
    }

    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);

    const int rc = sqlite3_step(stmt);
    const int changes = sqlite3_changes(db);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        throw std::runtime_error("[SqliteStore] Failed to delete device");
    }

    return changes > 0;
}

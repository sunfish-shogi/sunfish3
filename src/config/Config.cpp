/*
 * Config.cpp
 */

#include "Config.h"
#include "logger/Logger.h"
#include <sstream>
#include <fstream>

namespace sunfish {

void Config::addDef(const char* name, const char* defaultValue) {
  defs_[name] = { defaultValue };
}

void Config::addDef(const char* name, int32_t defaultValue) {
  addDef(name, std::to_string(defaultValue));
}

void Config::addDef(const char* name, bool defaultValue) {
  addDef(name, std::to_string(defaultValue));
}

bool Config::read(const char* filename) {
  Loggers::message << "open config file: " << filename;
  std::ifstream fin(filename);
  if (!fin) {
    Loggers::error << "open error: " << filename;
    return false;
  }

  data_.clear();

  for (int l = 0; ; l++) {
    char line[LINE_BUFFER_SIZE];
    fin.getline(line, sizeof(line));

    if (fin.eof()) {
      break;
    }

    if (fin.fail()) {
      Loggers::error << "read error: " << filename;
      return false;
    }

    if (!readLine(line)) {
      Loggers::error << "read error: " << filename << '(' << l << ')';
      Loggers::error << line;
    }
  }

  fin.close();

  return true;
}

bool Config::readLine(const std::string& line) {
  if (line[0] == '\0' || line[0] == '#') {
    return true;
  }

  auto sep = line.find_first_of('=');
  if (sep == std::string::npos) {
    return false;
  }
  std::string name = line.substr(0, sep);
  std::string value = line.substr(sep + 1);

  auto ite = defs_.find(name);

  if (ite == defs_.end()) {
    Loggers::error << __FILE_LINE__ << ": unknown key [" << name << "]";
    return false;
  }

  data_[name] = value;
  return true;
}

std::string Config::getString(const char* name) const {
  auto idat = data_.find(name);
  if (idat != data_.end()) {
    return idat->second;
  }

  auto idef = defs_.find(name);
  if (idef != defs_.end()) {
    return idef->second.defaultValue;
  }

  return "";
}

int32_t Config::getInt(const std::string& name) const {
  std::string value = getString(name);
  return std::stoi(value);
}

float Config::getFloat(const std::string& name) const {
  std::string value = getString(name);
  return std::stof(value);
}

double Config::getDouble(const std::string& name) const {
  std::string value = getString(name);
  return std::stod(value);
}

bool Config::getBool(const std::string& name) const {
  std::string value = getString(name);
  if (value == "true") {
    return true;
  } else if (value == "false") {
    return false;
  }
  return std::stoi(value) != 0;
}

std::string Config::toString() {
  std::ostringstream oss;
  for (const auto& pair : data_) {
    const auto& name = pair.first;
    const auto& value = pair.second;
    oss << name << "\t: " << value << '\n';
  }
  return oss.str();
}

} // namespace sunfish

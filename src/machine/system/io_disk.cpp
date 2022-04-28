#include "io_disk.hpp"

#include <string>
#include <tuple>
#include <unordered_map>
#include <fstream>
#include <filesystem>
#include <queue>
#include <mutex>
#include <memory>

namespace skiff {
namespace machine {
namespace system {

namespace {

enum class status_e {
  UNKNOWN_ERROR = 0,
  OKAY = 1,
  FILE_NOT_FOUND = 2,
  CANT_OPEN_FILE = 3,
  UNKNOWN_ID = 4,
};

enum class flags_e {
  INPUT,
  OUTPUT,
  BINARY,
  AT_END,
  TRUNC,
  APPEND
};

class file_c
{
public:
  file_c(const uint64_t id, const std::string file_path, const unsigned int flags) : 
    id{id}, _file_path{file_path}, _flags{flags} {
  }

  bool open();

  const bool is_open() { return _fs.is_open(); }
  void close() {  _fs.close(); }
  const bool is_read_only() { return _flags == std::fstream::in; }

  void write(const char * data, std::size_t len);

  uint64_t id{0};

private:
  std::fstream _fs;
  unsigned int _flags;
  std::string _file_path;
  std::mutex _mutex;
};

bool file_c::open()
{
  if (_fs.is_open()) { return true; }
  _fs.open(_file_path, _flags);
  return _fs.is_open();
}

void file_c::write(const char * data, std::size_t len)
{
  std::lock_guard<std::mutex> lock(_mutex);
  _fs.write(data, len);
}

} // End anonymous ns

class file_manager_c {
public:
  file_manager_c(){}

  // Open a file in a certain mode
  std::tuple<status_e, uint64_t> open(const unsigned int flags, const std::string file_path);

  // Close and remove a file
  std::tuple<status_e> close(const uint64_t id);

  bool nonblocking_id_exists(const uint64_t id);

  std::tuple<status_e, bool> output_string_to_file(const uint64_t id, const std::string data);
  std::tuple<status_e, bool> output_binary_to_file(const uint64_t id, const std::string data);

private:
  uint64_t _next_id{0};

  uint64_t get_an_id();
  bool id_exists(const uint64_t id);

  std::mutex _id_mutex;
  std::mutex _map_mutex;
  std::queue<uint64_t> _id_recycle_bin;
  std::unordered_map<uint64_t, std::unique_ptr<file_c>>  _file_map;
};

uint64_t file_manager_c::get_an_id()
{
  uint64_t id{0};
  {
    std::lock_guard<std::mutex> lock(_id_mutex);
    if (_id_recycle_bin.empty()) {
      id = _id_recycle_bin.front();
      _id_recycle_bin.pop();
    } else {
      id = _next_id++;
    }
  }
  return id;
}

bool file_manager_c::nonblocking_id_exists(const uint64_t id) 
{
  return _file_map.find(id) != _file_map.end();
}

bool file_manager_c::id_exists(const uint64_t id) 
{
  std::lock_guard<std::mutex> lock(_id_mutex);
  return _file_map.find(id) != _file_map.end();
}

std::tuple<status_e, uint64_t> file_manager_c::open(const unsigned int flags, const std::string file_path)
{
  if (!std::filesystem::is_regular_file(file_path)) {
    return { status_e::FILE_NOT_FOUND, 0 };
  }

  uint64_t id = get_an_id();
  file_c *file = new file_c(id, file_path, flags);

  if (!file->open()) {
    delete file;
    return { status_e::CANT_OPEN_FILE, 0};
  }

  // Lock the map mutex and insert the now opened file
  {
    std::lock_guard<std::mutex> lock(_map_mutex);
    _file_map.insert(std::make_pair(id, std::move(std::unique_ptr<file_c>(file))));
  }

  return { status_e::OKAY, id };
}

std::tuple<status_e> file_manager_c::close(const uint64_t id)
{
  std::lock_guard<std::mutex> lock(_id_mutex);
  auto it = _file_map.find(id);
  if (it == _file_map.end()){
    return {status_e::UNKNOWN_ID};
  }
  _file_map[id].get()->close();
  _file_map.erase(it);
  return {status_e::OKAY};
}

std::tuple<status_e, bool> file_manager_c::output_string_to_file(const uint64_t id, const std::string data)
{
  if(!nonblocking_id_exists(id)) {
    return {status_e::UNKNOWN_ID, false};
  }

  // Not blocking the map here on write so we don't lock all operations
  // might want to rethink how to do this. 

  _file_map[id].get()->write(data.c_str(), data.size());
  return {status_e::OKAY, true};
}

std::tuple<status_e, bool> file_manager_c::output_binary_to_file(const uint64_t id, const std::string data)
{
  if(!nonblocking_id_exists(id)) {
    return {status_e::UNKNOWN_ID, false};
  }

  // Not blocking the map here on write so we don't lock all operations
  // might want to rethink how to do this. 

  _file_map[id].get()->write(reinterpret_cast<const char *>(&data[0]), data.size());
  return {status_e::OKAY, true};
}

io_disk_c::io_disk_c() : _manager(new file_manager_c())
{
}

io_disk_c::~io_disk_c()
{
  delete _manager;
}

void io_disk_c::execute(skiff::types::view_t &view)
{

  // TODO:
  /*
      Figure out a way to encode the operations of creating / grabbing a file id
      and reading / writing it

      use _manager for everything
  
  
  
  */
}

}
}
}
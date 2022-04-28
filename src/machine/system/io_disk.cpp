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
  bool close(const uint64_t id);
  bool write(const uint64_t id, const char * data, std::size_t length);

private:
  uint64_t _next_id{0};

  std::mutex _mutex;
  std::queue<uint64_t> _id_recycle_bin;
  std::unordered_map<uint64_t, std::unique_ptr<file_c>>  _file_map;
};

std::tuple<status_e, uint64_t> file_manager_c::open(const unsigned int flags, const std::string file_path)
{
  std::lock_guard<std::mutex> lock(_mutex);

  uint64_t id{0};
  if (!_id_recycle_bin.empty()) {
    id = _id_recycle_bin.front();
    _id_recycle_bin.pop();
  } else {
    id = _next_id++;
  }

  file_c *file = new file_c(id, file_path, flags);

  if (!file->open()) {
    delete file;
    return { status_e::CANT_OPEN_FILE, 0};
  }

  _file_map.insert(std::make_pair(id, std::move(std::unique_ptr<file_c>(file))));

  return { status_e::OKAY, id };
}

bool file_manager_c::close(const uint64_t id)
{
  std::lock_guard<std::mutex> lock(_mutex);
  auto it = _file_map.find(id);
  if (it == _file_map.end()){
    return true;
  }
  _file_map[id].get()->close();
  _file_map.erase(it);
  _id_recycle_bin.push(id);
  return true;
}

bool file_manager_c::write(const uint64_t id, const char * data, std::size_t length)
{
  std::lock_guard<std::mutex> lock(_mutex);
  if (!data || length == 0) {
    return false;
  }
  if(_file_map.find(id) == _file_map.end()) {
    return false;
  }
  _file_map[id].get()->write(data, length);
  return true;
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
#include "io_disk.hpp"
#include "config.hpp"

#include <fstream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <tuple>
#include <vector>


#include <iostream>


namespace skiff {
namespace machine {
namespace system {

namespace {

enum class command_e {
  CREATE = 0,
  OPEN = 1,
  CLOSE = 2,
  WRITE = 3,
  READ = 4
};

class file_c {
public:
  file_c(const std::string file_path) : _file_path{file_path} {}

  bool open(const unsigned int flags);
  const bool is_open() { return _fs.is_open(); }
  void close() { _fs.close(); }
  void write(const char *data, std::size_t len);
  std::vector<uint8_t> read(const std::size_t len);

private:
  std::fstream _fs;
  std::string _file_path;
};

bool file_c::open(const unsigned int flags)
{
  if (_fs.is_open()) {
    return true;
  }
  _fs.open(_file_path, flags);
  return _fs.is_open();
}

void file_c::write(const char *data, std::size_t len) { _fs.write(data, len); }

std::vector<uint8_t> file_c::read(const std::size_t len)
{
  std::vector<uint8_t> in(len);
  _fs.read(reinterpret_cast<char *>(in.data()), len);
  return in;
}

} // namespace

class file_manager_c {
public:
  file_manager_c() {}
  ~file_manager_c()
  {
    for (auto i = 0; i < _files.size(); i++) {
      if (nullptr != _files.at(i)) {
        delete _files.at(i);
      }
    }
  }
  uint64_t create(const std::string file_path);
  bool remove(const uint64_t id);
  file_c *get_file(const uint64_t id);

private:
  uint64_t _next_id{0};
  std::mutex _mutex;
  std::queue<uint64_t> _id_recycle_bin;
  std::vector<file_c *> _files;
};

uint64_t file_manager_c::create(const std::string file_path)
{
  std::lock_guard<std::mutex> lock(_mutex);
  if (_id_recycle_bin.empty()) {
    _files.push_back(new file_c(file_path));
    return _files.size() - 1;
  }
  else {
    auto idx = _id_recycle_bin.front();
    _files[idx] = new file_c(file_path);
    _id_recycle_bin.pop();
    return idx;
  }
}

bool file_manager_c::remove(const uint64_t id)
{
  std::lock_guard<std::mutex> lock(_mutex);
  if (id >= _files.size() || nullptr == _files[id]) {
    return false;
  }

  if (_files.at(id)->is_open()) {
    return false;
  }

  _files.at(id)->close();
  delete _files.at(id);
  _files.at(id) = nullptr;
  _id_recycle_bin.push(id);
  return true;
}

file_c *file_manager_c::get_file(const uint64_t id)
{
  std::lock_guard<std::mutex> lock(_mutex);
  if (id >= _files.size() || nullptr == _files[id]) {
    return nullptr;
  }
  return _files.at(id);
}

io_disk_c::io_disk_c() : _manager(new file_manager_c()) {}

io_disk_c::~io_disk_c() { delete _manager; }

void io_disk_c::execute(skiff::types::view_t &view)
{
  std::cout << "execute disk | i0: " 
            << view.integer_registers[0] 
            << " i1: "
            << view.integer_registers[1]
            << std::endl;

  // Assume failure
  view.op_register = 0;

  auto slot = view.memory_manager.get_slot(view.integer_registers[0]);
  if (!slot) {
    return;
  }

  auto command_offset = view.integer_registers[1];
  auto [okay, command] = slot->get_word(command_offset);
  if (!okay) {
    return;
  }

  std::cout << "Command : " << command << std::endl;

  switch(static_cast<command_e>(command)) {
    case command_e::CREATE: return create(slot, view);
    case command_e::OPEN:   return open(slot, view);
    case command_e::CLOSE:  return close(slot, view);
    case command_e::WRITE: return write(slot, view);
    case command_e::READ: return read(slot, view);
  };

}

void io_disk_c::create(skiff::machine::memory::memory_c* slot, skiff::types::view_t &view)
{
  std::cout << "CREATE\n";

  // Skip the command word
  auto offset = view.integer_registers[1] + skiff::config::word_size_bytes;
  auto [path_okay, file_path_source_slot_id] = slot->get_qword(offset);
  if (!path_okay) {
    return;
  }

  std::cout << "Path source slot : " << file_path_source_slot_id << std::endl;

  offset += skiff::config::q_word_size_bytes;
  auto [offset_okay, file_path_source_slot_offset] = slot->get_qword(offset);
  if (!offset_okay) {
    return;
  }

  std::cout << "Path source offset: " << file_path_source_slot_offset << std::endl;

  offset += skiff::config::q_word_size_bytes;
  auto [len_okay, file_path_len] = slot->get_qword(offset);
  if (!len_okay) {
    return;
  }

  std::cout << "Path len: " << file_path_len << std::endl;


}

void io_disk_c::open(skiff::machine::memory::memory_c* slot, skiff::types::view_t &view)
{
  std::cout << "OPEN\n";
}

void io_disk_c::close(skiff::machine::memory::memory_c* slot, skiff::types::view_t &view)
{
  std::cout << "CLOSE\n";
}

void io_disk_c::write(skiff::machine::memory::memory_c* slot, skiff::types::view_t &view)
{
  std::cout << "WRITE\n";
}

void io_disk_c::read(skiff::machine::memory::memory_c* slot, skiff::types::view_t &view)
{
  std::cout << "READ\n";
}

} // namespace system
} // namespace machine
} // namespace skiff
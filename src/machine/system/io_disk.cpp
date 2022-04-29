#include "io_disk.hpp"
#include "config.hpp"

#include <fstream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <tuple>
#include <vector>


//#include <bitset>
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

  bool open(std::ios_base::openmode flags);
  const bool is_open() { return _fs.is_open(); }
  void close() { _fs.close(); }
  void write(const char *data, std::size_t len);
  std::vector<uint8_t> read(const std::size_t len);

private:
  std::fstream _fs;
  std::string _file_path;
};

bool file_c::open(std::ios_base::openmode flags)
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
  //std::cout << "CREATE\n";

  // Skip the command word
  auto offset = view.integer_registers[1] + skiff::config::word_size_bytes;
  auto [path_okay, file_path_source_slot_id] = slot->get_qword(offset);
  if (!path_okay) {
    return;
  }

  //std::cout << "Path source slot : " << file_path_source_slot_id << std::endl;

  offset += skiff::config::q_word_size_bytes;
  auto [offset_okay, file_path_source_slot_offset] = slot->get_qword(offset);
  if (!offset_okay) {
    return;
  }

  //std::cout << "Path source offset: " << file_path_source_slot_offset << std::endl;

  offset += skiff::config::q_word_size_bytes;
  auto [len_okay, file_path_len] = slot->get_qword(offset);
  if (!len_okay) {
    return;
  }

  //std::cout << "Path len: " << file_path_len << std::endl;

  auto path_slot = view.memory_manager.get_slot(file_path_source_slot_id);
  if (!path_slot) {
    return;
  }

  if (path_slot->size() < file_path_source_slot_offset + file_path_len) {
    return;
  }

  // Pull the path out of memory - this would be a lot faster if we had a 'slice' 
  // function on the memory object
  std::string path;
  for( int i = 0; i < file_path_len; i++) {
    auto [byte_okay, value] = path_slot->get_hword(file_path_source_slot_offset++);
    if (!byte_okay) {
      return;
    }
    path += static_cast<char>(value);
  }

  //std::cout << "Path: " << path << std::endl;

  // Set i0 to the file id and set op register to 1 to indicate success
  view.integer_registers[0] = _manager->create(path);
  view.op_register = 1;

  std::cout << "NEW FD: " << view.integer_registers[0] << std::endl;
}

void io_disk_c::open(skiff::machine::memory::memory_c* slot, skiff::types::view_t &view)
{
  //std::cout << "OPEN\n";
  auto offset = view.integer_registers[1] + skiff::config::word_size_bytes;
  auto [fd_okay, fd] = slot->get_qword(offset);
  if (!fd_okay) {
    return;
  }

  //std::cout << "FD: " << fd << std::endl;

  offset += skiff::config::q_word_size_bytes;
  auto [flags_okay, flags] = slot->get_word(offset);
  if (!flags_okay) {
    return;
  }

  //std::cout << std::bitset<8>(flags & 0xFF) << std::endl;

  auto file = _manager->get_file(fd);
  if (!file) {
    return;
  }

  view.op_register = static_cast<uint64_t>(file->open(static_cast<std::ios_base::openmode>(flags & 0xFF)));
}

void io_disk_c::close(skiff::machine::memory::memory_c* slot, skiff::types::view_t &view)
{
  std::cout << "CLOSE\n";

  auto offset = view.integer_registers[1] + skiff::config::word_size_bytes;
  auto [fd_okay, fd] = slot->get_qword(offset);
  if (!fd_okay) {
    return;
  }

  //std::cout << "FD: " << fd << std::endl;

  auto file = _manager->get_file(fd);
  if (!file) {
    return;
  }

  // Close the file
  file->close();

  // Remove it from the manager
  view.op_register = static_cast<uint64_t>(_manager->remove(fd));
}

void io_disk_c::write(skiff::machine::memory::memory_c* slot, skiff::types::view_t &view)
{
  //std::cout << "WRITE\n";
  auto offset = view.integer_registers[1] + skiff::config::word_size_bytes;
  auto [fd_okay, fd] = slot->get_qword(offset);
  if (!fd_okay) {
    return;
  }

  //std::cout << "FD: " << fd << std::endl;

  offset += skiff::config::q_word_size_bytes;
  auto [source_slot_okay, source_slot] = slot->get_qword(offset);
  if (!source_slot_okay) {
    return;
  }

  //std::cout << "SOURCE SLOT: " << source_slot << std::endl;

  offset += skiff::config::q_word_size_bytes;
  auto [source_offset_okay, source_offset] = slot->get_qword(offset);
  if (!source_offset_okay) {
    return;
  }
  
  //std::cout << "SOURCE OFFSET: " << source_offset << std::endl;

  offset += skiff::config::q_word_size_bytes;
  auto [len_okay, len] = slot->get_qword(offset);
  if (!len_okay) {
    return;
  }
  
 // std::cout << "SOURCE LEN: " << len << std::endl;

  auto ss = view.memory_manager.get_slot(source_slot);
  if (!ss) {
    return;
  }

  //std::cout << "Got source slot\n";

  if (ss->size() < source_offset + len) {
    return;
  }

  //std::cout << "Getting file\n";

  auto file = _manager->get_file(fd);
  if (!file) {
    return;
  }

 // std::cout << "Got file\n";
  // Pull the path out of memory - this would be a lot faster if we had a 'slice' 
  // function on the memory object
  std::vector<uint8_t> data;
  data.reserve(len);
  for( int i = 0; i < len; i++) {
    auto [byte_okay, value] = ss->get_hword(source_offset++);
    if (!byte_okay) {
      return;
    }
    data.push_back(value);
  }

  //std::cout << "Writing : " << reinterpret_cast<const char *>(&data[0]) << std::endl;

  // Write the data to disk
  file->write(reinterpret_cast<const char *>(&data[0]), data.size());

  view.op_register = 1;
}

void io_disk_c::read(skiff::machine::memory::memory_c* slot, skiff::types::view_t &view)
{
  //std::cout << "READ\n";
  auto offset = view.integer_registers[1] + skiff::config::word_size_bytes;
  auto [fd_okay, fd] = slot->get_qword(offset);
  if (!fd_okay) {
    return;
  }

  //std::cout << "FD: " << fd << std::endl;

  offset += skiff::config::q_word_size_bytes;
  auto [dest_slot_okay, dest_slot] = slot->get_qword(offset);
  if (!dest_slot_okay) {
    return;
  }

  //std::cout << "DEST SLOT: " << dest_slot << std::endl;

  offset += skiff::config::q_word_size_bytes;
  auto [dest_offset_okay, dest_offset] = slot->get_qword(offset);
  if (!dest_offset_okay) {
    return;
  }
  
  //std::cout << "DEST OFFSET: " << dest_offset << std::endl;

  offset += skiff::config::q_word_size_bytes;
  auto [len_okay, len] = slot->get_qword(offset);
  if (!len_okay) {
    return;
  }
  
  //std::cout << "DEST LEN: " << len << std::endl;

  auto ds = view.memory_manager.get_slot(dest_slot);
  if (!ds) {
    return;
  }

  //std::cout << "Got source slot\n";

  if (ds->size() < dest_offset + len) {
    return;
  }

  //std::cout << "Getting file\n";

  auto file = _manager->get_file(fd);
  if (!file) {
    return;
  }

  std::vector<uint8_t> data = file->read(len);
  //std::string str_in(reinterpret_cast<const char*>(data.data()), data.size());
  //std::cout << "Read : " << str_in << std::endl;

  for(auto b : data) {
    std::cout << (int) b << " ";

    if (!ds->put_hword(dest_offset++, b)) {
      return;
    }
  }
  std::cout << std::endl;


  view.op_register = data.size();
}

} // namespace system
} // namespace machine
} // namespace skiff
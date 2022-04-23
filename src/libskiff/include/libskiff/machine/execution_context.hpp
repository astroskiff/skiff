#ifndef SKIFF_EXECUTION_CONTEXT_HPP
#define SKIFF_EXECUTION_CONTEXT_HPP

#include "libskiff/types.hpp"

namespace libskiff {
namespace machine {

//! \brief FED of the executor_if used as a visitor pattern
class executor_if;

//! \brief Base level instruction
class instruction_c {
public:
  virtual ~instruction_c() = default;
  virtual void visit(executor_if &e) = 0;
};

class instruction_nop_c : public instruction_c {
public:
  virtual void visit(executor_if &e) override;
};

class instruction_exit_c : public instruction_c {
public:
  virtual void visit(executor_if &e) override;
};

class instruction_blt_c : public instruction_c {
public:
  instruction_blt_c(uint64_t dest, types::vm_register &lhs,
                    types::vm_register &rhs)
      : destination(dest), lhs_reg(lhs), rhs_reg(rhs)
  {
  }
  virtual void visit(executor_if &e) override;
  types::vm_register &lhs_reg;
  types::vm_register &rhs_reg;
  uint64_t destination;
};

class instruction_bgt_c : public instruction_c {
public:
  instruction_bgt_c(uint64_t dest, types::vm_register &lhs,
                    types::vm_register &rhs)
      : destination(dest), lhs_reg(lhs), rhs_reg(rhs)
  {
  }
  virtual void visit(executor_if &e) override;
  types::vm_register &lhs_reg;
  types::vm_register &rhs_reg;
  uint64_t destination;
};

class instruction_beq_c : public instruction_c {
public:
  instruction_beq_c(uint64_t dest, types::vm_register &lhs,
                    types::vm_register &rhs)
      : destination(dest), lhs_reg(lhs), rhs_reg(rhs)
  {
  }
  virtual void visit(executor_if &e) override;
  types::vm_register &lhs_reg;
  types::vm_register &rhs_reg;
  uint64_t destination;
};

class instruction_jmp_c : public instruction_c {
public:
  instruction_jmp_c(uint64_t dest) : destination(dest) {}
  virtual void visit(executor_if &e) override;
  uint64_t destination;
};

class instruction_call_c : public instruction_c {
public:
  instruction_call_c(uint64_t dest) : destination(dest) {}
  virtual void visit(executor_if &e) override;
  uint64_t destination;
};

class instruction_ret_c : public instruction_c {
public:
  virtual void visit(executor_if &e) override;
};

class instruction_mov_c : public instruction_c {
public:
  instruction_mov_c(types::vm_register &dest, uint64_t value)
      : dest_reg(dest), value(value)
  {
  }
  virtual void visit(executor_if &e) override;
  types::vm_register &dest_reg;
  uint64_t value;
};

class instruction_add_c : public instruction_c {
public:
  instruction_add_c(types::vm_register &dest, types::vm_register &lhs,
                    types::vm_register &rhs)
      : dest_reg(dest), lhs_reg(lhs), rhs_reg(rhs)
  {
  }
  virtual void visit(executor_if &e) override;
  types::vm_register &dest_reg;
  types::vm_register &lhs_reg;
  types::vm_register &rhs_reg;
};

class instruction_sub_c : public instruction_c {
public:
  instruction_sub_c(types::vm_register &dest, types::vm_register &lhs,
                    types::vm_register &rhs)
      : dest_reg(dest), lhs_reg(lhs), rhs_reg(rhs)
  {
  }
  virtual void visit(executor_if &e) override;
  types::vm_register &dest_reg;
  types::vm_register &lhs_reg;
  types::vm_register &rhs_reg;
};

class instruction_div_c : public instruction_c {
public:
  instruction_div_c(types::vm_register &dest, types::vm_register &lhs,
                    types::vm_register &rhs)
      : dest_reg(dest), lhs_reg(lhs), rhs_reg(rhs)
  {
  }
  virtual void visit(executor_if &e) override;
  types::vm_register &dest_reg;
  types::vm_register &lhs_reg;
  types::vm_register &rhs_reg;
};

class instruction_mul_c : public instruction_c {
public:
  instruction_mul_c(types::vm_register &dest, types::vm_register &lhs,
                    types::vm_register &rhs)
      : dest_reg(dest), lhs_reg(lhs), rhs_reg(rhs)
  {
  }
  virtual void visit(executor_if &e) override;
  types::vm_register &dest_reg;
  types::vm_register &lhs_reg;
  types::vm_register &rhs_reg;
};

class instruction_addf_c : public instruction_c {
public:
  instruction_addf_c(types::vm_register &dest, types::vm_register &lhs,
                     types::vm_register &rhs)
      : dest_reg(dest), lhs_reg(lhs), rhs_reg(rhs)
  {
  }
  virtual void visit(executor_if &e) override;
  types::vm_register &dest_reg;
  types::vm_register &lhs_reg;
  types::vm_register &rhs_reg;
};

class instruction_subf_c : public instruction_c {
public:
  instruction_subf_c(types::vm_register &dest, types::vm_register &lhs,
                     types::vm_register &rhs)
      : dest_reg(dest), lhs_reg(lhs), rhs_reg(rhs)
  {
  }
  virtual void visit(executor_if &e) override;
  types::vm_register &dest_reg;
  types::vm_register &lhs_reg;
  types::vm_register &rhs_reg;
};

class instruction_divf_c : public instruction_c {
public:
  instruction_divf_c(types::vm_register &dest, types::vm_register &lhs,
                     types::vm_register &rhs)
      : dest_reg(dest), lhs_reg(lhs), rhs_reg(rhs)
  {
  }
  virtual void visit(executor_if &e) override;
  types::vm_register &dest_reg;
  types::vm_register &lhs_reg;
  types::vm_register &rhs_reg;
};

class instruction_mulf_c : public instruction_c {
public:
  instruction_mulf_c(types::vm_register &dest, types::vm_register &lhs,
                     types::vm_register &rhs)
      : dest_reg(dest), lhs_reg(lhs), rhs_reg(rhs)
  {
  }
  virtual void visit(executor_if &e) override;
  types::vm_register &dest_reg;
  types::vm_register &lhs_reg;
  types::vm_register &rhs_reg;
};

class instruction_lsh_c : public instruction_c {
public:
  instruction_lsh_c(types::vm_register &dest, types::vm_register &lhs,
                    types::vm_register &rhs)
      : dest_reg(dest), lhs_reg(lhs), rhs_reg(rhs)
  {
  }
  virtual void visit(executor_if &e) override;
  types::vm_register &dest_reg;
  types::vm_register &lhs_reg;
  types::vm_register &rhs_reg;
};

class instruction_rsh_c : public instruction_c {
public:
  instruction_rsh_c(types::vm_register &dest, types::vm_register &lhs,
                    types::vm_register &rhs)
      : dest_reg(dest), lhs_reg(lhs), rhs_reg(rhs)
  {
  }
  virtual void visit(executor_if &e) override;
  types::vm_register &dest_reg;
  types::vm_register &lhs_reg;
  types::vm_register &rhs_reg;
};

class instruction_and_c : public instruction_c {
public:
  instruction_and_c(types::vm_register &dest, types::vm_register &lhs,
                    types::vm_register &rhs)
      : dest_reg(dest), lhs_reg(lhs), rhs_reg(rhs)
  {
  }
  virtual void visit(executor_if &e) override;
  types::vm_register &dest_reg;
  types::vm_register &lhs_reg;
  types::vm_register &rhs_reg;
};

class instruction_or_c : public instruction_c {
public:
  instruction_or_c(types::vm_register &dest, types::vm_register &lhs,
                   types::vm_register &rhs)
      : dest_reg(dest), lhs_reg(lhs), rhs_reg(rhs)
  {
  }
  virtual void visit(executor_if &e) override;
  types::vm_register &dest_reg;
  types::vm_register &lhs_reg;
  types::vm_register &rhs_reg;
};

class instruction_xor_c : public instruction_c {
public:
  instruction_xor_c(types::vm_register &dest, types::vm_register &lhs,
                    types::vm_register &rhs)
      : dest_reg(dest), lhs_reg(lhs), rhs_reg(rhs)
  {
  }
  virtual void visit(executor_if &e) override;
  types::vm_register &dest_reg;
  types::vm_register &lhs_reg;
  types::vm_register &rhs_reg;
};

class instruction_not_c : public instruction_c {
public:
  instruction_not_c(types::vm_register &dest, types::vm_register &source)
      : dest_reg(dest), source_reg(source)
  {
  }
  virtual void visit(executor_if &e) override;
  types::vm_register &dest_reg;
  types::vm_register &source_reg;
};

class instruction_bltf_c : public instruction_c {
public:
  instruction_bltf_c(uint64_t dest, types::vm_register &lhs,
                     types::vm_register &rhs)
      : destination(dest), lhs_reg(lhs), rhs_reg(rhs)
  {
  }
  virtual void visit(executor_if &e) override;
  types::vm_register &lhs_reg;
  types::vm_register &rhs_reg;
  uint64_t destination;
};

class instruction_bgtf_c : public instruction_c {
public:
  instruction_bgtf_c(uint64_t dest, types::vm_register &lhs,
                     types::vm_register &rhs)
      : destination(dest), lhs_reg(lhs), rhs_reg(rhs)
  {
  }
  virtual void visit(executor_if &e) override;
  types::vm_register &lhs_reg;
  types::vm_register &rhs_reg;
  uint64_t destination;
};

class instruction_beqf_c : public instruction_c {
public:
  instruction_beqf_c(uint64_t dest, types::vm_register &lhs,
                     types::vm_register &rhs)
      : destination(dest), lhs_reg(lhs), rhs_reg(rhs)
  {
  }
  virtual void visit(executor_if &e) override;
  types::vm_register &lhs_reg;
  types::vm_register &rhs_reg;
  uint64_t destination;
};

class instruction_aseq_c : public instruction_c {
public:
  instruction_aseq_c(types::vm_register &expected, types::vm_register &actual)
      : expected_reg(expected), actual_reg(actual)
  {
  }
  virtual void visit(executor_if &e) override;
  types::vm_register &expected_reg;
  types::vm_register &actual_reg;
};

class instruction_asne_c : public instruction_c {
public:
  instruction_asne_c(types::vm_register &expected, types::vm_register &actual)
      : expected_reg(expected), actual_reg(actual)
  {
  }
  virtual void visit(executor_if &e) override;
  types::vm_register &expected_reg;
  types::vm_register &actual_reg;
};

class instruction_push_w_c : public instruction_c {
public:
  instruction_push_w_c(types::vm_register &source) : source(source) {}
  virtual void visit(executor_if &e) override;
  types::vm_register &source;
};
class instruction_push_dw_c : public instruction_c {
public:
  instruction_push_dw_c(types::vm_register &source) : source(source) {}
  virtual void visit(executor_if &e) override;
  types::vm_register &source;
};
class instruction_push_qw_c : public instruction_c {
public:
  instruction_push_qw_c(types::vm_register &source) : source(source) {}
  virtual void visit(executor_if &e) override;
  types::vm_register &source;
};

class instruction_pop_w_c : public instruction_c {
public:
  instruction_pop_w_c(types::vm_register &dest) : dest(dest) {}
  virtual void visit(executor_if &e) override;
  types::vm_register &dest;
};

class instruction_pop_dw_c : public instruction_c {
public:
  instruction_pop_dw_c(types::vm_register &dest) : dest(dest) {}
  virtual void visit(executor_if &e) override;
  types::vm_register &dest;
};

class instruction_pop_qw_c : public instruction_c {
public:
  instruction_pop_qw_c(types::vm_register &dest) : dest(dest) {}
  virtual void visit(executor_if &e) override;
  types::vm_register &dest;
};

class instruction_alloc_c : public instruction_c {
public:
  instruction_alloc_c(types::vm_register &dest, types::vm_register &size)
      : dest(dest), size(size)
  {
  }
  virtual void visit(executor_if &e) override;
  types::vm_register &dest;
  types::vm_register &size;
};

class instruction_free_c : public instruction_c {
public:
  instruction_free_c(types::vm_register &idx) : idx(idx) {}
  virtual void visit(executor_if &e) override;
  types::vm_register &idx;
};

class instruction_store_word_c : public instruction_c {
public:
  instruction_store_word_c(types::vm_register &idx, types::vm_register &offset,
                           types::vm_register &data)
      : idx(idx), offset(offset), data(data)
  {
  }
  virtual void visit(executor_if &e) override;
  types::vm_register &idx;
  types::vm_register &offset;
  types::vm_register &data;
};

class instruction_store_dword_c : public instruction_c {
public:
  instruction_store_dword_c(types::vm_register &idx, types::vm_register &offset,
                            types::vm_register &data)
      : idx(idx), offset(offset), data(data)
  {
  }
  virtual void visit(executor_if &e) override;
  types::vm_register &idx;
  types::vm_register &offset;
  types::vm_register &data;
};

class instruction_store_qword_c : public instruction_c {
public:
  instruction_store_qword_c(types::vm_register &idx, types::vm_register &offset,
                            types::vm_register &data)
      : idx(idx), offset(offset), data(data)
  {
  }
  virtual void visit(executor_if &e) override;
  types::vm_register &idx;
  types::vm_register &offset;
  types::vm_register &data;
};

class instruction_load_word_c : public instruction_c {
public:
  instruction_load_word_c(types::vm_register &idx, types::vm_register &offset,
                          types::vm_register &dest)
      : idx(idx), offset(offset), dest(dest)
  {
  }
  virtual void visit(executor_if &e) override;
  types::vm_register &idx;
  types::vm_register &offset;
  types::vm_register &dest;
};

class instruction_load_dword_c : public instruction_c {
public:
  instruction_load_dword_c(types::vm_register &idx, types::vm_register &offset,
                           types::vm_register &dest)
      : idx(idx), offset(offset), dest(dest)
  {
  }
  virtual void visit(executor_if &e) override;
  types::vm_register &idx;
  types::vm_register &offset;
  types::vm_register &dest;
};

class instruction_load_qword_c : public instruction_c {
public:
  instruction_load_qword_c(types::vm_register &idx, types::vm_register &offset,
                           types::vm_register &dest)
      : idx(idx), offset(offset), dest(dest)
  {
  }
  virtual void visit(executor_if &e) override;
  types::vm_register &idx;
  types::vm_register &offset;
  types::vm_register &dest;
};

class instruction_syscall_c : public instruction_c {
public:
  instruction_syscall_c(const uint64_t &address) : address(address) {}
  virtual void visit(executor_if &e) override;
  uint64_t address;
};

class instruction_debug_c : public instruction_c {
public:
  instruction_debug_c(const uint64_t &id) : id(id) {}
  virtual void visit(executor_if &e) override;
  uint64_t id;
};

class instruction_eirq_c : public instruction_c {
public:
  instruction_eirq_c() {}
  virtual void visit(executor_if &e) override;
};

class instruction_dirq_c : public instruction_c {
public:
  instruction_dirq_c() {}
  virtual void visit(executor_if &e) override;
};

//! \brief Executor of instructions interface
class executor_if {
public:
  virtual void accept(instruction_nop_c &ins) = 0;
  virtual void accept(instruction_exit_c &ins) = 0;
  virtual void accept(instruction_blt_c &ins) = 0;
  virtual void accept(instruction_bgt_c &ins) = 0;
  virtual void accept(instruction_beq_c &ins) = 0;
  virtual void accept(instruction_jmp_c &ins) = 0;
  virtual void accept(instruction_call_c &ins) = 0;
  virtual void accept(instruction_ret_c &ins) = 0;
  virtual void accept(instruction_mov_c &ins) = 0;
  virtual void accept(instruction_add_c &ins) = 0;
  virtual void accept(instruction_sub_c &ins) = 0;
  virtual void accept(instruction_div_c &ins) = 0;
  virtual void accept(instruction_mul_c &ins) = 0;
  virtual void accept(instruction_addf_c &ins) = 0;
  virtual void accept(instruction_subf_c &ins) = 0;
  virtual void accept(instruction_divf_c &ins) = 0;
  virtual void accept(instruction_mulf_c &ins) = 0;
  virtual void accept(instruction_lsh_c &ins) = 0;
  virtual void accept(instruction_rsh_c &ins) = 0;
  virtual void accept(instruction_and_c &ins) = 0;
  virtual void accept(instruction_or_c &ins) = 0;
  virtual void accept(instruction_xor_c &ins) = 0;
  virtual void accept(instruction_not_c &ins) = 0;
  virtual void accept(instruction_bltf_c &ins) = 0;
  virtual void accept(instruction_bgtf_c &ins) = 0;
  virtual void accept(instruction_beqf_c &ins) = 0;
  virtual void accept(instruction_aseq_c &ins) = 0;
  virtual void accept(instruction_asne_c &ins) = 0;
  virtual void accept(instruction_push_w_c &ins) = 0;
  virtual void accept(instruction_push_dw_c &ins) = 0;
  virtual void accept(instruction_push_qw_c &ins) = 0;
  virtual void accept(instruction_pop_w_c &ins) = 0;
  virtual void accept(instruction_pop_dw_c &ins) = 0;
  virtual void accept(instruction_pop_qw_c &ins) = 0;
  virtual void accept(instruction_alloc_c &ins) = 0;
  virtual void accept(instruction_free_c &ins) = 0;
  virtual void accept(instruction_store_word_c &ins) = 0;
  virtual void accept(instruction_store_dword_c &ins) = 0;
  virtual void accept(instruction_store_qword_c &ins) = 0;
  virtual void accept(instruction_load_word_c &ins) = 0;
  virtual void accept(instruction_load_dword_c &ins) = 0;
  virtual void accept(instruction_load_qword_c &ins) = 0;
  virtual void accept(instruction_syscall_c &ins) = 0;
  virtual void accept(instruction_debug_c &ins) = 0;
  virtual void accept(instruction_eirq_c &ins) = 0;
  virtual void accept(instruction_dirq_c &ins) = 0;
};

} // namespace machine
} // namespace libskiff

#endif

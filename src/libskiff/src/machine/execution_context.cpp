#include "libskiff/machine/execution_context.hpp"

namespace libskiff {
namespace machine {

/*
    Instruction `accept` implementations
*/
void instruction_nop_c::visit(executor_if &e) { e.accept(*this); }
void instruction_exit_c::visit(executor_if &e) { e.accept(*this); }
void instruction_blt_c::visit(executor_if &e) { e.accept(*this); }
void instruction_bgt_c::visit(executor_if &e) { e.accept(*this); }
void instruction_beq_c::visit(executor_if &e) { e.accept(*this); }
void instruction_jmp_c::visit(executor_if &e) { e.accept(*this); }
void instruction_call_c::visit(executor_if &e) { e.accept(*this); }
void instruction_ret_c::visit(executor_if &e) { e.accept(*this); }
void instruction_mov_c::visit(executor_if &e) { e.accept(*this); }
void instruction_add_c::visit(executor_if &e) { e.accept(*this); }
void instruction_sub_c::visit(executor_if &e) { e.accept(*this); }
void instruction_div_c::visit(executor_if &e) { e.accept(*this); }
void instruction_mul_c::visit(executor_if &e) { e.accept(*this); }
void instruction_addf_c::visit(executor_if &e) { e.accept(*this); }
void instruction_subf_c::visit(executor_if &e) { e.accept(*this); }
void instruction_divf_c::visit(executor_if &e) { e.accept(*this); }
void instruction_mulf_c::visit(executor_if &e) { e.accept(*this); }
void instruction_lsh_c::visit(executor_if &e) { e.accept(*this); }
void instruction_rsh_c::visit(executor_if &e) { e.accept(*this); }
void instruction_and_c::visit(executor_if &e) { e.accept(*this); }
void instruction_or_c::visit(executor_if &e) { e.accept(*this); }
void instruction_xor_c::visit(executor_if &e) { e.accept(*this); }
void instruction_not_c::visit(executor_if &e) { e.accept(*this); }
void instruction_bltf_c::visit(executor_if &e) { e.accept(*this); }
void instruction_bgtf_c::visit(executor_if &e) { e.accept(*this); }
void instruction_beqf_c::visit(executor_if &e) { e.accept(*this); }
void instruction_aseq_c::visit(executor_if &e) { e.accept(*this); }
void instruction_asne_c::visit(executor_if &e) { e.accept(*this); }
void instruction_push_w_c::visit(executor_if &e) { e.accept(*this); }
void instruction_push_dw_c::visit(executor_if &e) { e.accept(*this); }
void instruction_push_qw_c::visit(executor_if &e) { e.accept(*this); }
void instruction_pop_w_c::visit(executor_if &e) { e.accept(*this); }
void instruction_pop_dw_c::visit(executor_if &e) { e.accept(*this); }
void instruction_pop_qw_c::visit(executor_if &e) { e.accept(*this); }
void instruction_alloc_c::visit(executor_if &e) { e.accept(*this); }
void instruction_free_c::visit(executor_if &e) { e.accept(*this); }
void instruction_store_word_c::visit(executor_if &e) { e.accept(*this); }
void instruction_store_dword_c::visit(executor_if &e) { e.accept(*this); }
void instruction_store_qword_c::visit(executor_if &e) { e.accept(*this); }
void instruction_load_word_c::visit(executor_if &e) { e.accept(*this); }
void instruction_load_dword_c::visit(executor_if &e) { e.accept(*this); }
void instruction_load_qword_c::visit(executor_if &e) { e.accept(*this); }

} // namespace machine
} // namespace libskiff
/* !don't remove! -*- C++ -*-
 *
 * Teng -- a general purpose templating engine.
 * Copyright (C) 2004  Seznam.cz, a.s.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Seznam.cz, a.s.
 * Naskove 1, Praha 5, 15000, Czech Republic
 * http://www.seznam.cz, mailto:teng@firma.seznam.cz
 *
 *
 * $Id: tengsyntax.yy,v 1.14 2010-06-11 08:25:35 burlog Exp $
 *
 * DESCRIPTION
 * Teng grammar semantic actions.
 *
 * AUTHORS
 * Stepan Skrob <stepan@firma.seznam.cz>
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-06-07  (burlog)
 *             Moved from syntax.yy.
 */

#ifndef TENGSEMANTIC_H
#define TENGSEMANTIC_H

#include <string>

#include "tengerror.h"
#include "tenglogging.h"
#include "tengprogram.h"
#include "tengparsercontext.h"
#include "tenginstruction.h"

namespace Teng {
namespace Parser {

/** Saves error position for further processing.
 */
Token_t note_error(Context_t *ctx, const Token_t &token);

/** Clears last stored error.
 */
void reset_error(Context_t *ctx);

/** Saves the point where expression begins.
 */
void note_expr_start_point(Context_t *ctx, const Pos_t &pos);

/** Saves the highest valid address (program->size() - 1) as possible
 * optimization point.
 */
void note_optimization_point(Context_t *ctx, bool optimizable);

/** Generates code for variable.
 */
void generate_var(Context_t *ctx, Variable_t var);

/** Prepare Variable_t symbol in context var_sym temporary for absolute
 * variable that starts with '.';
 */
void prepare_root_variable(Context_t *ctx, const Token_t &token);

/** Prepare Variable_t symbol in context var_sym temporary for absolute
 * variable that starts with '_this';
 */
void prepare_this_variable(Context_t *ctx, const Token_t &token);

/** Prepare Variable_t symbol in context var_sym temporary for absolute
 * variable that starts with '_parent';
 */
void prepare_parent_variable(Context_t *ctx, const Token_t &token);

/** Generates print instruction.
 */
void generate_print(Context_t *ctx, bool print_escape = true);

/** Generates as many as needed open frag instructions.
 */
void open_frag(Context_t *ctx, const Pos_t &pos, Variable_t &frag);

/** Warns about invalid fragment name.
 */
void open_inv_frag(Context_t *ctx, const Pos_t &pos);

/** Generates as many as needed close frag instructions.
 */
void close_frag(Context_t *ctx, const Pos_t &pos, bool invalid = false);

/** Closes invalid fragment.
 */
void close_inv_frag(Context_t *ctx, const Pos_t &pos);

/** Generates as many as needed close frag instructions.
 */
void
close_unclosed_frag(Context_t *ctx, const Pos_t &pos, const Token_t &token);

/** Attempts to optimize expression during compile time. (NOT TRUE -> FALSE)
 */
void optimize_expr(Context_t *ctx, uint32_t arity, bool lazy_evaluated = false);

/** Generates instruction for given value.
 */
void generate_val(Context_t *ctx, const Pos_t &pos, Value_t value);

/** Generates lookup to dictionary instruction.
 */
void generate_dict_lookup(Context_t *ctx, const Token_t &token);

/** Discards all instructions generated by valid part of invalid expression and
 * restore context to valid state.
 */
void discard_expr(Context_t *ctx);

/** Cleanes used resources for building exprs.
 */
void finish_expr(Context_t *ctx);

/** Generates first conditional instruction for ternary operator.
 */
void generate_tern_op(Context_t *ctx, const Token_t &token);

/** Finalizes jumps for true branch of ternary operator.
 */
void finalize_tern_op_true_branch(Context_t *ctx, const Token_t &token);

/** Finalizes jumps for false branch of ternary operator.
 */
void finalize_tern_op_false_branch(Context_t *ctx);

/** Generates runtime variable instruction from variable identifier for query
 * call.
 */
void generate_query(Context_t *ctx, const Variable_t &var, bool warn);

/** Writes invalid query variable warning into error log.
 */
void invalid_query(Context_t *ctx, const Token_t &token);

/** Opens given file and replace include directive with content of the file.
 */
void include_file(Context_t *ctx, const Pos_t &pos, const Options_t &opts);

/** Writes warning into log error about ignored include.
 */
void ignore_include(Context_t *ctx, const Token_t &token, bool empty = false);

/** Prepares case expression.
 */
void prepare_case(Context_t *ctx);

/** Prepares condition of the case expression.
 */
void prepare_case_cond(Context_t *ctx, const Token_t &token);

/** Generates comparison for case branch.
 */
uint32_t generate_case_cmp(Context_t *ctx, Literal_t &literal);

/** Updates jmp address offset in Jmp_t instruction generated for case
 * alternative value.
 */
void update_case_jmp(Context_t *ctx, const Token_t &token, uint32_t alts);

/** Generates alternative matching instructions for case option alternative.
 */
uint32_t generate_case_next(Context_t *ctx, Literal_t &literal, uint32_t alts);

/** Updates jmp address offset in JmpIfNot_t instruction generated for result
 * of matching case option.
 */
void finalize_case_branch(Context_t *ctx, const Token_t &token);

/** The case expression is a bit complicated so it takes a lot instruction to
 * build it. Consider such expression and, for the sake of simplicity, split it
 * to several parts.
 *
 * case(1, 1: 'first', 2, 3: 'second', *: 'default')
 *
 * 1. The case expression condition
 *
 * The condition is an expression, here numeric literal 1, that has to be pushed
 * onto value stack:
 *
 * 000 VAL                 <value=1>
 * 001 PUSH
 *
 * 2. The label of first case branch
 *
 * The case label has to be literal and should be compared with case
 * 'condition' value. These instructions implement such algorithm.
 *
 * 002 STACK               <index=0>
 * 003 VAL                 <value=1>
 * 004 EQ
 * 005 JMPIFNOT            <jump=-1>
 *
 * The STACK instruction push the top of value stack onto compute stack. The EQ
 * takes two values from compute stack and pushes back the result of comparison.
 * If comparison returns true then JMPIFNOT instruction does not perform jump
 * and case branch for current label is executed.
 *
 * 006 VAL                 <value='first'>
 * 007 JMP                 <jump=-1>
 *
 * The last instruction is jump at the end of case expr.
 *
 * 3. The label of second case branch
 *
 * The second label has two alternatives. They are implementated as ORing of
 * two comparison as in previous label.
 *
 * 008 STACK               <index=0>
 * 009 VAL                 <value=2>
 * 010 EQ
 * 011 OR                  <jump=+4>
 * 012 STACK               <index=0>
 * 013 VAL                 <value=3>
 * 014 EQ
 * 015 JMPIFNOT            <jump=-1>
 * 016 VAL                 <value='second'>
 * 017 JMP                 <jump=-1>
 *
 * 4. The default label of case
 *
 * If one label match the case condition value then the instruction in default
 * branch are executed.
 *
 * 018 VAL                 <value='ostatni'>
 *
 * 5. Finalizing
 *
 * You could noticed that jump instruction has set invalid offsets. They can't
 * be calculated during generating instructions because the offset aren't
 * known at the time. Therefore, the ctx->branch_addrs stack of jpm
 * instructions is used to store the addresses of all instructions, and the
 * offsets are immediately updated as they are known.
 *
 * Finally, the case condition value has to be pop out from value stack.
 *
 * 019 POP
 */
NAryExpr_t
finalize_case(Context_t *ctx, const Token_t &token, uint32_t arity);

/** Inserts new diagnostic code into diag-codes storage. If pop is set to true
 * then the previous diagnostic code is poped out.
 *
 * The diagnostic codes are used to help the template writer where the syntax
 * error probably is.
 */
void expr_diag(Context_t *ctx, diag_code_type new_diag_code, bool pop = true);

/** Generates instructions implementing format switching.
 */
void open_format(Context_t *ctx, const Pos_t &pos, const Options_t &opts);

/** Generates instructions implementing format switching.
 */
void open_inv_format(Context_t *ctx, const Pos_t &pos);

/** Generates instructions that restore previous (or default) format.
 */
void close_format(Context_t *ctx, const Pos_t &pos);

/** Generates instructions that restore previous (or default) format and
 * reports invalid token.
 */
void close_inv_format(Context_t *ctx, const Pos_t &pos);

/** Generates instructions that restore previous (or default) format and
 * reports missing close format directive.
 */
void
close_unclosed_format(Context_t *ctx, const Pos_t &pos, const Token_t &token);

/** Generates instructions implementing content type switching.
 */
void open_ctype(Context_t *ctx, const Pos_t &pos, const Literal_t &type);

/** Generates instructions implementing content type switching.
 */
void open_inv_ctype(Context_t *ctx, const Pos_t &pos);

/** Generates instructions that restore previous (or default) content type.
 */
void close_ctype(Context_t *ctx, const Pos_t &pos);

/** Generates instructions that restore previous (or default) content type and
 * reports invalid token.
 */
void close_inv_ctype(Context_t *ctx, const Pos_t &pos);

/** Generates instructions that restore previous (or default) content type and
 * reports missing close content type directive.
 */
void
close_unclosed_ctype(Context_t *ctx, const Pos_t &pos, const Token_t &token);

/** Prepares new if statement.
 */
void prepare_if(Context_t *ctx, const Pos_t &pos);

/** Generates instructions implementing the if expression.
 */
void generate_if(Context_t *ctx, const Token_t &token, bool valid_expr);

/** Generates instructions implementing the if expression.
 */
void generate_if(Context_t *ctx, const Token_t &token, const Token_t &inv);

/** Calculates if expression jump and updates appropriate instructions in
 * program.
 */
void finalize_if_branch(Context_t *ctx, int32_t shift);

/** Updates branch terminating jumps.
 */
void finalize_if(Context_t *ctx);

/** Updates branch terminating jumps.
 */
void finalize_inv_if(Context_t *ctx, const Pos_t &pos);

/** Generates instructions implementing the else expression.
 */
void generate_else(Context_t *ctx, const Token_t &token);

/** Generates instructions implementing the else expression.
 */
void generate_inv_else(Context_t *ctx, const Token_t &token);

/** Calculates if expression's jump and updates appropriate instructions in
 * program.
 */
void generate_elif(Context_t *ctx, const Token_t &token);

/** Cleans after if statement building.
 */
void finalize_if_stmnt(Context_t *ctx);

/** Cleans after invalid if statement building.
 */
void finalize_inv_if_stmnt(Context_t *ctx, const Token_t &token);

/** Discards while if statement because of invalid order of else/elif branches.
 */
void discard_if(Context_t *ctx);

/** Generates code implementing the setting variable.
 */
void set_var(Context_t *ctx, Variable_t var_name);

/** Generates warning about invalid variable name.
 */
void ignore_inv_set(Context_t *ctx, const Pos_t &pos);

/** Generates code implementing runtime variable indexing.
 */
void generate_rtvar_index(Context_t *ctx, const Token_t &lp, const Token_t &rp);

/** Generates code implementing regex.
 */
Regex_t generate_regex(Context_t *ctx, const Token_t &regex);

/** Generates code implementing regex matching.
 */
void generate_match(Context_t *ctx, const Token_t &token, const Token_t &regex);

/** Generates code implementing debug fragment.
 */
void debug_frag(Context_t *ctx, const Pos_t &pos, bool warn = false);

/** Generates code implementing bytecode fragment.
 */
void bytecode_frag(Context_t *ctx, const Pos_t &pos, bool warn = false);

/** Generates warning with unknown Teng directive message.
 */
void ignore_unknown_directive(Context_t *ctx, const Token_t &token);

/** Generates warning about excessive options in directive that does not accept
 * any.
 */
void ignore_excessive_options(Context_t *ctx, const Pos_t &pos);

/** Inserts new option to options list. It expects that ctx->opts_sym is valid
 * symbol (not moved out).
 */
void new_option(Context_t *ctx, const Token_t &name, Literal_t &&literal);

/** Prepares new expression.
 */
void prepare_expr(Context_t *ctx, const Pos_t &pos);

/** Generates instructions implementing getting value of runtime variable for
 * desired key.
 */
void
generate_rtvar_segment(Context_t *ctx, const Token_t &token, bool is_first);

/** Generates instructions implementing getting this fragment.
 */
void generate_rtvar_this(Context_t *ctx, const Token_t &token, bool is_first);

/** Generates instructions implementing getting parent fragment.
 */
void generate_rtvar_parent(Context_t *ctx, const Token_t &token, bool is_first);

/** Inserts new diagnostic code into diag-codes storage including the diag code
 * sentinel.
 */
inline void expr_diag_sentinel(Context_t *ctx, diag_code_type new_diag_code) {
    ctx->expr_diag.push_sentinel();
    expr_diag(ctx, new_diag_code, false);
}

/** Generates given instruction pass given args to instruction c'tor.
 */
template <typename Instr_t, typename... Args_t>
void generate(Context_t *ctx, Args_t &&...args) {
    ctx->program->emplace_back<Instr_t>(std::forward<Args_t>(args)...);
}

/** Generates expression from given symbol.
 */
template <typename Instr_t>
void generate_expr(Context_t *ctx, const Token_t &token) {
    generate<Instr_t>(ctx, token.pos);
}

/** Generates expression from given symbol.
 */
template <typename Instr_t>
uint32_t generate_str_expr(Context_t *ctx, const Token_t &token) {
    // TODO(burlog): delete this when STR_EQ/STR_NE is over
    if (!ctx->program->empty()) {
        auto &instr = ctx->program->back();
        if (instr.opcode() == OPCODE::VAL) {
            auto &value = instr.as<Val_t>().value;
            if (value.is_regex()) {
                Regex_t regex = std::move(value.as_regex());
                ctx->program->pop_back();
                generate<RegexMatch_t>(ctx, std::move(regex), token.pos);
                ctx->optimization_points.pop();
                if (std::is_same<Instr_t, StrNE_t>::value)
                    generate<Not_t>(ctx, token.pos);
                return 1;
            }
        }
    }
    generate_expr<Instr_t>(ctx, token);
    return 2;
}

/** Generates raw print instruction.
 */
inline void generate_raw_print(Context_t *ctx) {
    generate_print(ctx, false);
}

/** Generates print instruction of given token value.
 */
inline void generate_raw_print(Context_t *ctx, const Token_t &token) {
    generate_val(ctx, token.pos, Value_t(token.str()));
    generate_raw_print(ctx);
}

/** Generates print instruction of given token value. It expects that value is
 * "undefined" which does not need escaping.
 */
inline void generate_inv_print(Context_t *ctx, const Token_t &inv) {
    note_error(ctx, inv);
    reset_error(ctx);
    logWarning(ctx, inv.pos, "Invalid expression; the behaviour is undefined");
    generate_raw_print(ctx);
}

/** Generates binary operator (OR, AND, ...).
 */
template <typename Instr_t>
void generate_bin_op(Context_t *ctx, const Token_t &token) {
    ctx->branch_addrs.top().push(ctx->program->size());
    generate<Instr_t>(ctx, token.pos);
}

/** Finalizes binary operator (OR, AND, ...).
 */
template <typename Instr_t>
void finalize_bin_op(Context_t *ctx) {
    int32_t bin_op_addr = ctx->branch_addrs.top().pop();
    auto addr_offset = ctx->program->size() - bin_op_addr - 1;
    (*ctx->program)[bin_op_addr].as<Instr_t>().addr_offset = addr_offset;
}

/** Generates instructions implementing runtime variable root.
 */
template <typename Instr_t>
void generate_rtvar(Context_t *ctx, const Token_t &token) {
    uint16_t root_offset = ctx->open_frames.top().size();
    generate<Instr_t>(ctx, root_offset, token.pos);
    note_optimization_point(ctx, true);
    if (std::is_same<Instr_t, PushThisFrag_t>::value)
        ctx->rtvar_strings.emplace_back(token.view().begin(), 0);
    else ctx->rtvar_strings.push_back(token.view());
}

/** Generates instructions implementing local runtime variable.
 */
void generate_local_rtvar(Context_t *ctx, const Token_t &token);

/** Generates instructions implementing query expression.
 * If arity is not 1 then query is badly formated and instruction is not
 * generated.
 */
template <typename Instr_t>
NAryExpr_t query_expr(Context_t *ctx, const Token_t &token, uint32_t arity) {
    if (std::is_same<Instr_t, QueryDefined_t>::value) {
        logWarning(
            ctx,
            token.pos,
            "The defined() query is deprecated; "
            "use isempty() or exists() instead"
        );
    } else if (std::is_same<Instr_t, QueryCount_t>::value) {
        logWarning(
            ctx,
            token.pos,
            "The count() query is deprecated; "
            "use _count builtin variable instead"
        );
    }
    if (arity == 1) {
        generate<Instr_t>(ctx, token.pos);
    } else {
        logError(
            ctx,
            token.pos,
            "Invalid variable identifier in " + token.view() + "()"
        );
        generate_val(ctx, token.pos, Value_t());
    }
    return NAryExpr_t(token, arity);
}


/** Generates instructions implementing the function call.
 */
uint32_t generate_func(Context_t *ctx, const Token_t &name, uint32_t nargs);

/** Generates lookup to dictionary instruction.
 */
inline void print_dict_lookup(Context_t *ctx, const Token_t &token) {
    generate_dict_lookup(ctx, token);
    generate_raw_print(ctx);
}

/** Generates undefined value due to invalid dict identifier.
 */
inline void print_dict_undef(Context_t *ctx, const Token_t &token) {
    generate_val(ctx, token.pos, Value_t());
    generate_raw_print(ctx);
    logWarning(ctx, token.pos, "Invalid dictionary key in #{...} statement");
}

/** Generates nice warning abount ignored _this.
 */
inline void ignoring_this(Context_t *ctx, const Pos_t &pos) {
    logWarning(ctx, pos, "Ignoring useless '_this' variable path segment");
}

/** Generates nice warning abount ignored dollar.
 */
inline void obsolete_dollar(Context_t *ctx, const Pos_t &pos) {
    logWarning(ctx, pos, "Don't use dollar sign here please");
}

} // namespace Parser
} // namespace Teng

#endif /* TENGSEMANTIC_H */

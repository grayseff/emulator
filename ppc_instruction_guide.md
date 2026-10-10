# PowerPC Emulator Instruction Guide

This is a plain-English guide to the instruction subset currently
implemented by the emulator. It is meant to be read alongside
`instruction.h`, `decode.c`, and `execute.c`, rather than as a complete
PowerPC reference manual.

## 1. The machine state these instructions operate on

### General-purpose registers (GPRs)

The CPU has 32 general-purpose 32-bit registers, `r0` through `r31`. In
the emulator these are:

``` c
cpu->gpr[0] ... cpu->gpr[31]
```

Instruction fields such as `ra`, `rb`, `rd`, `rs`, and `rx` are usually
**register numbers**, not the values themselves. For example, if
`ra == 5`, the value of that operand is `cpu->gpr[5]`.

PowerPC uses different names for the same encoded five-bit field
depending on its role:

-   **RA, RB** - register operands, although RA is also often a
    destination in logical/rotate instructions.
-   **RD / RT** - destination/target register.
-   **RS** - source register.
-   **RX** - our neutral emulator name where the same encoded field has
    different roles in different instructions.

### PC - Program Counter

`pc` identifies the instruction being executed / next instruction
address according to the convention used by our `step()` implementation.
Ordinary execution advances through memory one 4-byte instruction at a
time. Branch instructions replace that normal flow with another address.

### CR - Condition Register

The 32-bit Condition Register is divided into eight 4-bit fields:

``` text
CR0 CR1 CR2 CR3 CR4 CR5 CR6 CR7
```

A normal integer result recorded into a CR field is represented by four
flags:

``` text
LT  GT  EQ  SO
```

-   **LT** - result/left operand was less than the comparison value.
-   **GT** - greater than.
-   **EQ** - equal / result was zero.
-   **SO** - summary overflow, copied from XER.

`CR0` is simply the first of those eight fields. Many integer
instructions can optionally describe their result in CR0.

### Rc - Record bit

Some instruction encodings contain an `Rc` bit. If `Rc == 1`, the
instruction records information about its result in `CR0`.

For example, `add.` is effectively ADD with `Rc = 1`. If the result is
negative, CR0 gets LT; if positive, GT; if zero, EQ. SO is copied from
XER.

This is why our ADD implementation performs the arithmetic
unconditionally but only calls `set_cr0()` when `ins.op.x.rc` is true.

### XER and SO

XER is the Fixed-Point Exception Register. Our current subset barely
models it. The part currently used is **SO**, Summary Overflow, at bit
31.

When we update a CR field, our helper copies XER.SO into that field's SO
bit. We do not yet implement the full range of XER behaviour.

### LR - Link Register

The Link Register is primarily used for branches and function calls.

A branch with `LK = 1` saves the return address in LR. A later `bclr`
can branch back to the address in LR. This is the basic machinery behind
ordinary PPC call/return sequences.

### CTR - Count Register

CTR is another special-purpose register. Conditional branches can
decrement it and decide whether to branch based on whether it has
reached zero. This makes counted loops efficient.

### Immediate values

An **immediate** is a number stored directly inside the instruction
rather than fetched from a register.

Our decoder deliberately keeps immediate fields raw. `execute.c` decides
whether an instruction treats one as signed, unsigned, shifted, etc.

A signed 16-bit immediate such as `0xffff` means `-1`, so
`sign_extend16()` expands it to the equivalent 32-bit bit pattern
`0xffffffff`.

### Effective addresses

Loads and stores need a memory address.

D-form memory instructions use roughly:

``` text
address = (RA == 0 ? 0 : GPR[RA]) + signed immediate
```

Indexed X-form memory instructions use:

``` text
address = (RA == 0 ? 0 : GPR[RA]) + GPR[RB]
```

The `U` suffix means **update**: after using the calculated address, the
instruction also writes that address back into RA.

The `X` suffix means **indexed**: the second part of the address comes
from RB instead of an immediate.

------------------------------------------------------------------------

# 2. Arithmetic instructions

## ADD - Add

``` text
ADD RD, RA, RB
```

Adds the 32-bit values in registers RA and RB and puts the result in RD.

``` text
GPR[RD] = GPR[RA] + GPR[RB]
```

If `Rc` is set, the result is also described in CR0.

## ADDI - Add Immediate

``` text
ADDI RD, RA, immediate
```

Sign-extends the 16-bit immediate and adds it to RA, storing the result
in RD.

``` text
GPR[RD] = base + signed(immediate)
```

For ADDI specifically, an RA field of zero means the base is literal
zero rather than `GPR[0]`. This makes ADDI useful for loading small
signed constants.

## ADDIS - Add Immediate Shifted

Like ADDI, except the sign-extended immediate is shifted left by 16 bits
before addition.

``` text
GPR[RD] = base + (signed(immediate) << 16)
```

This is useful for constructing values whose interesting bits are in the
upper half of a 32-bit register. The common `lis` assembler mnemonic is
a simplified form of ADDIS with RA = 0.

## SUBF - Subtract From

``` text
SUBF RD, RA, RB
```

Despite the slightly counterintuitive name, this computes:

``` text
GPR[RD] = GPR[RB] - GPR[RA]
```

Think **subtract RA from RB**.

If `Rc` is set, the result is recorded in CR0.

------------------------------------------------------------------------

# 3. Logical instructions

These operate independently on corresponding bits.

## AND - Bitwise AND

PowerPC syntax is:

``` text
AND RA, RS, RB
```

Each result bit is 1 only if the corresponding bits in RS and RB are
both 1.

``` text
GPR[RA] = GPR[RS] & GPR[RB]
```

If `Rc` is set, the result is recorded in CR0.

**Implementation note:** this operand arrangement matters. In the X-form
encoding used here, bits 21-25 are RS for logical instructions and bits
16-20 are the destination RA. A generic field name such as `rd` can
therefore be misleading.

## ANDI - AND Immediate

``` text
ANDI RA, RS, immediate
```

ANDs RS with the unsigned 16-bit immediate and writes the result to RA.

Unlike most immediate logical instructions, `andi.` is inherently a
recording instruction in PowerPC, so CR0 is updated.

## ANDIS - AND Immediate Shifted

Same as ANDI, except the immediate occupies the upper 16 bits:

``` text
GPR[RA] = GPR[RS] & (immediate << 16)
```

CR0 is updated.

## OR - Bitwise OR

``` text
OR RA, RS, RB
```

A result bit is 1 if that bit is 1 in either source.

``` text
GPR[RA] = GPR[RS] | GPR[RB]
```

If `Rc` is set, CR0 is updated.

A useful consequence is that ORing a register with itself copies it.

## ORI - OR Immediate

ORs a register with an unsigned 16-bit immediate.

``` text
GPR[RA] = GPR[RS] | immediate
```

`ori r0,r0,0` is the conventional PPC no-op.

## ORIS - OR Immediate Shifted

Like ORI, but shifts the immediate into the upper 16 bits first.

``` text
GPR[RA] = GPR[RS] | (immediate << 16)
```

## XOR - Exclusive OR

``` text
XOR RA, RS, RB
```

A result bit is 1 when the corresponding source bits are different.

``` text
GPR[RA] = GPR[RS] ^ GPR[RB]
```

If `Rc` is set, CR0 is updated.

## XORI - XOR Immediate

XORs RS with the unsigned 16-bit immediate and stores the result in RA.

## XORIS - XOR Immediate Shifted

As XORI, except the immediate is shifted left 16 bits before the XOR.

------------------------------------------------------------------------

# 4. Compare instructions

Compare instructions do not produce a normal GPR result. They describe
the relationship between two values in one of the eight CR fields
selected by `BF`.

## CMP - Compare Signed

Treats RA and RB as signed 32-bit integers.

The selected CR field becomes:

-   LT if RA \< RB
-   GT if RA \> RB
-   EQ if RA == RB
-   SO copied from XER

## CMPL - Compare Logical

The word **logical** here effectively means **unsigned comparison**.

RA and RB are compared as values from 0 through 0xffffffff rather than
as signed two's-complement integers.

## CMPI - Compare Immediate Signed

Compares RA as a signed 32-bit integer with a sign-extended 16-bit
immediate, storing LT/GT/EQ in the selected CR field.

## CMPLI - Compare Logical Immediate

Compares RA with an unsigned 16-bit immediate and stores the result in
the selected CR field.

------------------------------------------------------------------------

# 5. Branch instructions

## B - Branch

Changes execution to another instruction address.

The encoded LI field represents a signed branch displacement with two
implicit zero low bits, because PPC instructions are 4-byte aligned.

-   `AA = 0`: target is relative to the address of the branch
    instruction.
-   `AA = 1`: target is an absolute address derived from the signed
    displacement.
-   `LK = 1`: save the address of the following instruction in LR.

A branch with LK set is therefore usable as a function call.

## BC - Branch Conditional

BC adds two controls, `BO` and `BI`.

`BI` selects a particular bit in the Condition Register.

`BO` controls whether the branch should test that CR bit, use/decrement
CTR, and what outcome it wants from those tests.

In plain English, BC can express ideas such as:

``` text
branch if equal
branch if not equal
decrement CTR and branch if it is not zero
```

The branch happens only if the requested conditions are satisfied.

Its BD displacement is signed and, like B, has two implicit zero low
bits.

## BCLR - Branch Conditional to Link Register

Tests a branch condition similarly to BC, but if the condition succeeds,
the destination comes from LR rather than from an encoded displacement.

The low two bits of LR are cleared for the branch target.

This instruction is commonly used to return from a function. The
familiar `blr` is a simplified mnemonic for a particular BCLR
configuration.

If LK is set, LR is also updated with the following-instruction address.

------------------------------------------------------------------------

# 6. Loads - memory to registers

Naming pieces:

-   **LB** - load byte (8 bits)
-   **LH** - load halfword (16 bits)
-   **LW** - load word (32 bits)
-   **Z** - zero extend
-   **A** - algebraic, i.e. signed/sign-extended
-   **U** - update RA with the effective address
-   **X** - indexed address using RB

## LWZ - Load Word and Zero

Calculates `RA + signed immediate`, reads 32 bits from memory, and puts
them in the target register.

For a 32-bit GPR, the "zero" part is not very exciting because the
loaded word already fills the register.

## LWZU - Load Word and Zero with Update

Does LWZ, then stores the effective address back into RA.

## LBZ - Load Byte and Zero

Reads one byte and places it in the target GPR with all upper bits zero.

Example:

``` text
memory byte = 0xff
register    = 0x000000ff
```

## LBZU - Load Byte and Zero with Update

LBZ plus writing the effective address back into RA.

## LHZ - Load Halfword and Zero

Reads 16 bits and places them in the target GPR with the upper 16 bits
zero.

## LHZU - Load Halfword and Zero with Update

LHZ plus updating RA.

## LHA - Load Halfword Algebraic

Reads 16 bits and **sign-extends** them to 32 bits.

Example:

``` text
memory halfword = 0xffff
register        = 0xffffffff
```

so a stored signed `-1` remains `-1` after loading.

## LHAU - Load Halfword Algebraic with Update

LHA plus updating RA.

------------------------------------------------------------------------

# 7. Stores - registers to memory

## STW - Store Word

Calculates `RA + signed immediate` and writes all 32 bits of the source
GPR to memory.

## STWU - Store Word with Update

STW plus writing the effective address back into RA.

## STB - Store Byte

Writes only the low 8 bits of the source register to memory.

## STBU - Store Byte with Update

STB plus updating RA.

## STH - Store Halfword

Writes only the low 16 bits of the source register to memory.

## STHU - Store Halfword with Update

STH plus updating RA.

------------------------------------------------------------------------

# 8. Indexed loads and stores

These perform the same basic operations as the preceding loads/stores,
but calculate:

``` text
effective address = base + GPR[RB]
```

instead of using a 16-bit immediate.

The base is zero when the encoded RA field is zero.

## LWZX

Load a 32-bit word from `RA + RB`.

## LWZUX

LWZX and then update RA with the effective address.

## LBZX

Load an 8-bit byte from `RA + RB` and zero-extend it.

## LBZUX

LBZX and then update RA.

## LHZX

Load a 16-bit halfword from `RA + RB` and zero-extend it.

## LHZUX

LHZX and then update RA.

## LHAX

Load a 16-bit halfword from `RA + RB` and sign-extend it to 32 bits.

## LHAUX

LHAX and then update RA.

## STWX

Store all 32 bits of the source GPR at `RA + RB`.

## STWUX

STWX and then update RA.

## STBX

Store the low 8 bits of the source GPR at `RA + RB`.

## STBUX

STBX and then update RA.

## STHX

Store the low 16 bits of the source GPR at `RA + RB`.

## STHUX

STHX and then update RA.

------------------------------------------------------------------------

# 9. Special-purpose register instructions

## MFSPR - Move From Special-Purpose Register

Copies a special-purpose register into a normal GPR.

Our emulator currently supports:

``` text
SPR 1 -> XER
SPR 8 -> LR
SPR 9 -> CTR
```

So MFSPR can retrieve the current XER, link address, or loop/count
value.

## MTSPR - Move To Special-Purpose Register

The reverse of MFSPR: copies a normal GPR into a selected
special-purpose register.

Our current subset supports XER, LR, and CTR.

------------------------------------------------------------------------

# 10. Rotate-and-mask instructions

These initially look horrible, but all three are variations on one idea:

1.  rotate a 32-bit value left;
2.  construct a mask from MB (mask begin) and ME (mask end);
3.  keep or insert the selected bits.

A **rotate** differs from an ordinary shift because bits that fall off
the left end wrap around to the right end.

PowerPC numbers bits from the most-significant end for these mask
fields, which is why the C mask helper may initially look backwards.

## RLWINM - Rotate Left Word Immediate then AND with Mask

Takes RS, rotates it left by the immediate shift amount, ANDs the
rotated value with the MB/ME mask, and stores the result in RA.

``` text
RA = rotate_left(RS, SH) & mask(MB, ME)
```

This one instruction can implement many common shifts, bit extractions,
and masking operations.

If Rc is set, CR0 describes the result.

## RLWIMI - Rotate Left Word Immediate then Mask Insert

Rotates RS, but instead of replacing all of RA it replaces only the bits
selected by the mask.

Conceptually:

``` text
RA = (old_RA & ~mask) | (rotated_RS & mask)
```

Bits outside the mask survive unchanged.

If Rc is set, CR0 describes the final RA value.

## RLWNM - Rotate Left Word then AND with Mask

Like RLWINM, except the rotation amount comes from a register rather
than directly from the instruction.

Only the low five bits of that register matter, giving a rotation from 0
through 31.

------------------------------------------------------------------------

# 11. UNKNOWN

`UNKNOWN` is our decoder's safe representation for an instruction
encoding we do not support.

Eventually this should become a proper guest illegal-instruction/program
exception. At present the executor merely reports it.

------------------------------------------------------------------------

# 12. Reading the suffixes at a glance

A useful decoding vocabulary:

``` text
I       immediate value encoded in instruction
IS      immediate shifted left 16
X       indexed: address uses another register
U       update: write effective address back to RA
Z       zero extend loaded value
A       algebraic/signed load
L       logical/unsigned comparison
Rc      record result in CR0 when supported
LK      save return address in Link Register
AA      branch target is absolute rather than relative
```

Once these become familiar, names such as `LHZUX` are much less
mysterious:

``` text
Load Halfword + Zero extend + Update + indeXed
```

------------------------------------------------------------------------

# 13. Notes from reviewing the current execute.c

The overall C structure is good: a small collection of genuinely
reusable `static` helpers followed by one direct opcode switch. That
matches the simple style of the rest of the emulator better than
creating one function per instruction.

There are, however, several semantic details to fix or verify before
treating this implementation as authoritative:

1.  **AND / OR / XOR register roles.** PPC uses `RA, RS, RB`: RA is the
    destination and the encoded 21-25 field is RS. The current executor
    treats the 21-25 field (`x.rd`) as the destination. The decoder/form
    design should represent or account for this distinction.

2.  **Immediate logical register roles.** `andi.`, `andis.`, `ori`,
    `oris`, `xori`, and `xoris` also use `RA` as destination and `RS` in
    the 21-25 field. A generic D-form `rx` is fine, but execution must
    interpret `rx` as RS and `ra` as the destination for these logical
    instructions. The current code does the opposite.

3.  **Branch displacement sign extension.** B's LI field and BC's BD
    field are signed displacements. Because the decoder intentionally
    stores raw fields, execution must sign-extend the appropriate width
    before/while appending the two zero alignment bits. The current
    generic `branch()` simply shifts the raw field left by two.

4.  **B and BC have different displacement widths.** LI is 24 encoded
    bits while BD is 14 encoded bits. Passing both raw values to one
    helper without telling it the width cannot correctly sign-extend
    both.

5.  **BCLR target alignment.** A BCLR target is derived from LR with its
    low two bits cleared. The current code assigns the saved LR value
    directly to PC.

6.  **Branch/PC convention needs one explicit check.** The current
    helper contains a `-4` adjustment for relative branches. Whether
    that is correct depends on exactly when `step()` increments PC. Pick
    one convention ("PC still points at current instruction during
    execute" or "PC already points at next instruction") and make all
    B/BC/BCLR/LK behaviour follow it consistently.

7.  **Update-form legality is intentionally unfinished.** The TODOs for
    cases such as RA = 0 or RA = target are reasonable placeholders
    until guest program exceptions are modeled.

These are good reasons to treat the old Rust implementation as a
behavioural starting point, not as the PowerPC specification.

// Example of reading Bytecode Dump in C
// This example prints all protos of input file
// Usage: example_c [INPUT_FILE]

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define BCDUMP_F_BE     0x01
#define BCDUMP_F_STRIP  0x02
#define BCDUMP_F_FFI    0x04

#define BCDUMP_F_KNOWN  (BCDUMP_F_FFI*2-1)

#define mask_op (0xFF)
#define mask_a (0xFF<<8)
#define mask_c (0xFF<<16)
#define mask_b (0xFF<<24)
#define mask_d (0xFFFF<<16)

#define get_op(v) (BCOp)(v&mask_op)
#define get_a(v) (byte_t)((v&mask_a)>>8)
#define get_c(v) (byte_t)((v&mask_c)>>16)
#define get_b(v) (byte_t)((v&mask_b)>>24)
#define get_d(v) (uint16_t)((v&mask_d)>>16)

typedef enum {
   BC_ISLT, BC_ISGE, BC_ISLE, BC_ISGT, BC_ISEQV, BC_ISNEV, BC_ISEQS, BC_ISNES, BC_ISEQN, BC_ISNEN,
   BC_ISEQP, BC_ISNEP, BC_ISTC, BC_ISFC, BC_IST, BC_ISF, BC_MOV, BC_NOT, BC_UNM, BC_LEN, BC_ADDVN,
   BC_SUBVN, BC_MULVN, BC_DIVVN, BC_MODVN, BC_ADDNV, BC_SUBNV, BC_MULNV, BC_DIVNV, BC_MODNV, BC_ADDVV,
   BC_SUBVV, BC_MULVV, BC_DIVVV, BC_MODVV, BC_POW, BC_CAT, BC_KSTR, BC_KCDATA, BC_KSHORT, BC_KNUM,
   BC_KPRI, BC_KNIL, BC_UGET, BC_USETV, BC_USETS, BC_USETN, BC_USETP, BC_UCLO, BC_FNEW, BC_TNEW,
   BC_TDUP, BC_GGET, BC_GSET, BC_TGETV, BC_TGETS, BC_TGETB, BC_TSETV, BC_TSETS, BC_TSETB, BC_TSETM,
   BC_CALLM, BC_CALL, BC_CALLMT, BC_CALLT, BC_ITERC, BC_ITERN, BC_VARG, BC_ISNEXT, BC_RETM, BC_RET,
   BC_RET0, BC_RET1, BC_FORI, BC_JFORI, BC_FORL, BC_IFORL, BC_JFORL, BC_ITERL, BC_IITERL, BC_JITERL,
   BC_LOOP, BC_ILOOP, BC_JLOOP, BC_JMP, BC_FUNCF, BC_IFUNCF, BC_JFUNCF, BC_FUNCV, BC_IFUNCV, BC_JFUNCV,
   BC_FUNCC, BC_FUNCCW,
   BC__MAX
} BCOp;
enum {
  BCDUMP_KGC_CHILD, BCDUMP_KGC_TAB, BCDUMP_KGC_I64, BCDUMP_KGC_U64,
  BCDUMP_KGC_COMPLEX, BCDUMP_KGC_STR
};
enum {
  BCDUMP_KTAB_NIL, BCDUMP_KTAB_FALSE, BCDUMP_KTAB_TRUE,
  BCDUMP_KTAB_INT, BCDUMP_KTAB_NUM, BCDUMP_KTAB_STR
};
typedef unsigned int Ins;
typedef unsigned char byte_t;
typedef struct sbytecode {
   byte_t* top; // Pointer to top of bytecode
   byte_t* cur; // Cursor
   size_t size; // Bytecode size
   int flags; // Bytecode flags
} bytecode;

const char* BC_NAMES = "ISLT  ISGE  ISLE  ISGT  ISEQV ISNEV ISEQS ISNES ISEQN ISNEN ISEQP ISNEP ISTC  ISFC  IST   ISF   MOV   NOT   UNM   LEN   ADDVN SUBVN MULVN DIVVN MODVN ADDNV SUBNV MULNV DIVNV MODNV ADDVV SUBVV MULVV DIVVV MODVV POW   CAT   KSTR  KCDATAKSHORTKNUM  KPRI  KNIL  UGET  USETV USETS USETN USETP UCLO  FNEW  TNEW  TDUP  GGET  GSET  TGETV TGETS TGETB TSETV TSETS TSETB TSETM CALLM CALL  CALLMTCALLT ITERC ITERN VARG  ISNEXTRETM  RET   RET0  RET1  FORI  JFORI FORL  IFORL JFORL ITERL IITERLJITERLLOOP  ILOOP JLOOP JMP   FUNCF IFUNCFJFUNCFFUNCV IFUNCVJFUNCVFUNCC FUNCCW";
const char* get_opcode_name(BCOp opcode) {
   if (opcode >= BC__MAX) {
      char* buff = (char*)malloc(14);
      sprintf(buff, "UNKNOWN<%d>\0", opcode);
      return buff;
   }
   const char* const_op_name = BC_NAMES+(opcode*6);
   char* op_name = (char*)malloc(7);
   memcpy(op_name, const_op_name, 7);
   op_name[6] = '\0';
   return op_name;
}

#define ClearMemory(p,s) memset(p,0,s)
#define BoolToStr(b) (b ? "true" : "false")

#define read_byte(bc) (*bc->cur++)
#define read_short(bc) (*(uint16_t)(bc->cur)++)
int read_uleb128(bytecode* bc) {
   int v = read_byte(bc);
   if (v >= 0x80) {
      v &= 0x7f;
      int off = 1;
      do {
         v |= ((read_byte(bc) & 0x7f) << (off * 7));
         off++;
      } while (*bc->cur >= 0x80);
   }
   return v;
}
int read_uleb128_33(bytecode* bc) {
   int v = read_byte(bc) >> 1;
   if (v >= 0x40) {
      v &= 0x3f;
      int off = -1;
      do {
         v |= ((read_byte(bc) & 0x7f) << (off += 7));
      } while (*bc->cur >= 0x80);
   }
}
byte_t* read_mem(bytecode* bc, int size) {
   byte_t* p = (byte_t*)malloc(size);
   memcpy(p, bc->cur, size);
   bc->cur += size;
   return p;
}
const char* read_buffer(bytecode* bc, int size) {
   char* buff = (char*)read_mem(bc, size+1);
   bc->cur--;
   buff[size] = '\0';
   return buff;
}

void read_ktabk(bytecode* bc) {
   int tp = read_uleb128(bc);
   if (tp >= BCDUMP_KTAB_STR) {
      int len = tp - BCDUMP_KTAB_STR;
      read_buffer(bc, len);
   } else if (tp == BCDUMP_KTAB_INT) {
      read_uleb128(bc);
   } else if (tp == BCDUMP_KTAB_NUM) {
      read_uleb128(bc);
      read_uleb128(bc);
   }
}

void read_kgci(bytecode* bc, const int i) {
   int tp = read_uleb128(bc);
   if (tp >= BCDUMP_KGC_STR) {
      int len = tp - BCDUMP_KGC_STR;
      const char* buff = read_buffer(bc, len);
      printf("CONST\t%d\tSTRING\t\"%s\"\n", i, buff);
   } else if (tp == BCDUMP_KGC_TAB) {
      printf("CONST\t%d\tTABLE\n", i);

      int karray_len = read_uleb128(bc);
      int khash_len = read_uleb128(bc);

      for (int karr_i; karr_i < karray_len; karr_i++) {
         read_ktabk(bc);
      }
      for (int khash_i; khash_i < khash_len; khash_i++) {
         read_ktabk(bc);
         read_ktabk(bc);
      }
   } else if (tp == BCDUMP_KGC_CHILD) {
      printf("CONST\t%d\tCHILD\n", i);
   } else {
      int low = read_uleb128(bc), high = read_uleb128(bc);
      long long v = ((long long)(low)<<32) | ((long long)(high));
      if (tp == BCDUMP_KGC_COMPLEX) {
         int ilow = read_uleb128(bc), ihigh = read_uleb128(bc);
         long long vh = ((long long)(ilow)<<32) | ((long long)(ihigh));
         printf("CONST\t%d\tINT128<low32=%ld;high32=%ld>\n", i, v, vh);
      } else {
         printf("CONST\t%d\tINT64<%ld>\n", i, v);
      }
   }
}

void read_proto(bytecode* bc) {
   int proto_len = read_uleb128(bc); 
   int pflags = read_byte(bc);
   int numparams = read_byte(bc);
   int framesize = read_byte(bc);
   int sizeuv = read_byte(bc);
   int sizekgc = read_uleb128(bc);
   int sizekn = read_uleb128(bc);
   int sizebc = read_uleb128(bc);

   int sizedbg = 0;
   if (!(bc->flags & BCDUMP_F_STRIP)) {
      sizedbg = read_uleb128(bc);
      int firstline = read_uleb128(bc);
      int numline = read_uleb128(bc);
      printf("Lines info: First=%d, Number=%d\n", firstline, numline);
   }

   Ins* code = (Ins*)read_mem(bc, sizebc * sizeof(Ins));
   uint16_t* uvdata = (uint16_t*)read_mem(bc, sizeuv * sizeof(uint16_t));

   for (int i = 0; i < sizekgc; i++) {
      read_kgci(bc, i);
   }
   for (int i = 0; i < sizekn; i++) {
      byte_t is64 = (*bc->cur) & 1;
      int low = read_uleb128_33(bc);
      if (is64) {
         int high = read_uleb128(bc);
         long long v = ((long long)(low) << 32) | high;
         printf("KCONST\t%d\tINT64<%ld>\n", i, v);
      } else {
         printf("KCONST\t%d\tINT32<%d>\n", i, low);
      }
   }

   if (!(bc->flags & BCDUMP_F_STRIP)) {
      read_mem(bc, sizedbg);
   }
   printf("\n");

   /* Print bytecode */
   for (size_t i = 0; i < sizebc; i++) {
      Ins inst = code[i];

      BCOp op = get_op(inst);
      byte_t a = get_a(inst);
      byte_t b = get_b(inst);
      byte_t c = get_c(inst);
      uint16_t d = get_d(inst);

      const char* op_name = get_opcode_name(op);

      printf("%.4d\t%s\tA=%d\tB=%d\tC=%d\tD=%d\n", i, op_name, a, b, c, d);
   }
}

int main(int argc, char* argv[]) {
   if (argc < 2) {
      fprintf(stderr, "Error: Expected input file\n");
      fflush(stderr);
      return 1;
   }
   
   FILE* f = fopen(argv[1], "r");
   if (f == NULL) {
      fprintf(stderr, "Error: Invalid input file\n");
      fflush(stderr);
      return 1;
   }
   
   fseek(f, 0, SEEK_END);
   long file_size = ftell(f);
   fseek(f, 0, SEEK_SET);

   byte_t* filec = (byte_t*)malloc(file_size);
   ClearMemory(filec, file_size);

   fread(filec, sizeof(byte_t), file_size, f);
   fclose(f);

   bytecode* bc = (bytecode*)malloc(sizeof(bytecode));
   ClearMemory(bc, sizeof(bytecode));

   bc->top = filec;
   bc->cur = bc->top;
   bc->size = file_size;

   /* Check magic */
   if (  
      (bc->cur[0]) != 0x1B ||
      (bc->cur[1]) != 0x4C || 
      (bc->cur[2]) != 0x4A
   ) {
      fprintf(stderr, "Error: Invalid bytecode format\n");
      fflush(stderr);
      return 1;
   }
   bc->cur += 3;

   byte_t version = read_byte(bc);

   printf("Loading script with version %d...\n", version);

   int flags = read_uleb128(bc);
   bc->flags = flags;

   printf("Big-Endian: %s\n", BoolToStr((flags & BCDUMP_F_BE) != 0));
   printf("Strip debug: %s\n", BoolToStr((flags & BCDUMP_F_STRIP) != 0));
   printf("FFI: %s\n", BoolToStr((flags & BCDUMP_F_FFI) != 0));

   const char* chunkname = NULL;
   if (!(flags & BCDUMP_F_STRIP)) {
      int chunkname_len = read_uleb128(bc);
      chunkname = read_buffer(bc, chunkname_len);
   }
   printf("Chunk name: %s\n", chunkname);

   printf("Reading protos...\n");
   for (int proto_id = 0; (*bc->cur) != 0; proto_id++) {
      printf("____________________ PROTO %d ____________________\n", proto_id);
         read_proto(bc);
      printf("_________________________________________________\n");
   }
   printf("Exiting...\n");
   return 0;  
}
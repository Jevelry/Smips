// COMP1521 20T2 --- assignment 2 - smips, Simple MIPS
// Written by <<Z5311917>>, August 2020
#include <stdio.h> 
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

// Bit values 
#define INSTRUCTION_HIGH_BIT 31
#define INSTRUCTION_LOW_BIT 26
#define registers_S_HIGH_BIT 25
#define registers_S_LOW_BIT  21
#define registers_T_HIGH_BIT  20
#define registers_T_LOW_BIT  16
#define registers_D_HIGH_BIT  15
#define registers_D_LOW_BIT  11
#define IMMEDIATE_HIGH_BIT  15
#define IMMEDIATE_LOW_BIT  0

// Max number instructions allowed
#define MAX_LINES 1000

// Instruction bit values
#define MUL 28
#define BEQ 4
#define BNE 5
#define ADDI 8
#define SLTI 10
#define ANDI 12
#define ORI 13
#define LUI 15

#define ADD 32
#define SUB 34
#define AND 36
#define OR 37
#define SLT 42
#define SYSCALL 12

// Funcstions
uint32_t extract_bit_range (uint32_t value, int high, int low);
void decode_instruction_set1 (int bit_instruction);
void decode_instruction_set2 (int bit_instruction);
int check_valid_instruction (int bit_instruction);
void run_instruction_set1 (int bit_instruction, int *pc, int *registers);
void run_instruction_set2 (int bit_instruction, int *pc, int *registers);
void print_registers (int *registers);
int decode_registers_d (int bit_instruction);
int decode_registers_s (int bit_instruction);
int decode_registers_t (int bit_instruction);
short decode_immediate (int bit_instruction);


int main(int argc, char *argv[]) {
    
    FILE *f = fopen(argv[1], "r");
    
    // Check if file open succeeded  
    if (f == NULL) {
        printf("No such file or directory: '%s'\n", argv[1]);      
        return 1;
    }

    // Initialise array storage for instructions
    int instruction_lines[MAX_LINES];
    int n_instructions = 0;
    
    //Scan instruction codes into array
    while (n_instructions < MAX_LINES && fscanf(f, "%x", 
        &instruction_lines[n_instructions]) == 1) {        
        n_instructions++;
    }
    
    // Loop, check if each instruction is valid, error = 1 if invalid instruction read
    int error = 0;
    int which_line = 0;
    while (which_line < n_instructions) {
        error = check_valid_instruction (instruction_lines[which_line]);
        if (error == 1) {
            printf("%s:%d: invalid instruction code: %08X\n", argv[1], 
                   which_line + 1, instruction_lines[which_line]);
            return 0;
        }
        which_line++;
    }
    
    printf("Program\n");
 
    // Loop through, decode each instruction code and print it    
    for (int i = 0; i < n_instructions; i++) {        
        printf("%3d: ", i);    
        // Seperate instructions into 2 groups based on if first 6 bits are 0                               
        if (extract_bit_range(instruction_lines[i], INSTRUCTION_HIGH_BIT,     
            INSTRUCTION_LOW_BIT) == 0) {   
            // Instructions to decode: add, sub, and, or , slt, syscall                                         
            decode_instruction_set1 (instruction_lines[i]);                                                                
        } else {
            // Instructions to decode: beq, bne, addi, slti, andi, ori, lui 
            decode_instruction_set2 (instruction_lines[i]);       
        }      
    }
    
    printf("Output\n");
    
    // Initialise registers, program counter and pointer to PC
    int registers[32] = {0};
    int program_counter = 0;   
    int *pc;
    pc = &program_counter;       
    // Loop through, run each instruction, exit if PC = -1 (syscall exit)
    while (program_counter < n_instructions && program_counter != -1) {
        // Seperate instructions into 2 groups based on if first 6 bits are 0
        if (extract_bit_range(instruction_lines[program_counter], INSTRUCTION_HIGH_BIT,     
            INSTRUCTION_LOW_BIT) == 0) {   
            // Instructions  to run: add, sub, and, or , slt, syscall 
            run_instruction_set1 (instruction_lines[program_counter], pc, registers);                                         
       } else {
            // Instructions to run: beq, bne, addi, slti, andi, ori, lui 
            run_instruction_set2 (instruction_lines[program_counter], pc, registers);       
        }              
    }    
    printf("Registers After Execution\n");
    
    // Print value of registers with non-zero value
    print_registers(registers);
                                       
    return 0;
}

///////////////////////////////////////////////////////////////
/////                        FUNCTIONS


// Extract a certain range of bits from a 32bit int (code taken from lecture example)
uint32_t extract_bit_range(uint32_t value, int high, int low) {
    uint32_t mask = (((uint32_t)1) << (high - low + 1)) - 1;
    return (value >> low) & mask;
}

// Check if given a hex number not corresponding to an instruction
int check_valid_instruction (int bit_instruction) {
    // Read the bits of instruction, check if they match bit pattern
    if (bit_instruction && SYSCALL == SYSCALL) {
        return 0;
    }
    // Read first 6 bits, check if they match instruction bit pattern     
    int instruction = extract_bit_range(bit_instruction, INSTRUCTION_HIGH_BIT, INSTRUCTION_LOW_BIT);
    if (instruction == MUL || instruction == BEQ || instruction == BNE ||
        instruction == ADDI || instruction == SLTI || instruction == ANDI ||
        instruction == ORI || instruction == LUI) {
        return 0;
    } 
    // If instruction code is within last 6 bits;
    if (instruction == 0) {
        // Read last 6 bits, check if they match instruction bit pattern
        instruction = extract_bit_range (bit_instruction, 5, 0);
        if (instruction == ADD || instruction == SUB || instruction == AND ||
            instruction == OR || instruction == SLT) {
            return 0;
        }
    }
    // If no pattern matched, return error = 1
    return 1;    
}

// Reads in 32 bit instruction and output 
//"The instruction (assempler) corresponding to each instruction code"
void decode_instruction_set1 (int bit_instruction) {
    
    // Extract value of registers from bit pattern
    int d = decode_registers_d (bit_instruction);
    int s = decode_registers_s (bit_instruction);
    int t = decode_registers_t (bit_instruction);
    // Extract last 6 bits from bit pattern (the instruction)
    uint32_t which_instruction = extract_bit_range(bit_instruction, 5, 0);
    char *instruction;
    // Identify MIPS instruction from last 6 bits
    if (which_instruction == ADD) {
        instruction = "add";
    } else if (which_instruction == SUB) {
         instruction = "sub";
    } else if (which_instruction == AND) {
        instruction = "and";
    } else if (which_instruction == OR) {
         instruction = "or";
    } else if (which_instruction == SLT) {
         instruction = "slt";
    } else {
        instruction = "syscall";
        printf("%s\n",instruction);
        return;
    }
    // i.e "add  $t1, $t2, $t3\n"
    printf("%s  $%d, $%d, $%d\n", instruction, d, s, t);
    return;
}

// Reads in 32 bit instruction and outputs 
//"The instruction (assempler) corresponding to each instruction code"
void decode_instruction_set2 (int bit_instruction) {
    
    // Extract value of registers and immediate value from bit pattern
    int s = decode_registers_s (bit_instruction);
    int t = decode_registers_t (bit_instruction);
    short I = decode_immediate (bit_instruction);
    int d = decode_registers_d (bit_instruction);
    
    // Extract first 6 bits from bit pattern (the instruction)
    uint32_t which_instruction = extract_bit_range(bit_instruction, INSTRUCTION_HIGH_BIT, INSTRUCTION_LOW_BIT);
    
    char *instruction;
    // Identify MIPS instruction and print appropriate command
    if (which_instruction == MUL) {
        instruction = "mul";
        printf("%s  $%d, $%d, $%d\n", instruction, d, s, t);
    } else if (which_instruction == BEQ) {
         instruction = "beq";
         printf("%s  $%d, $%d, %d\n", instruction, s, t, I);
    } else if (which_instruction == BNE) {
        instruction = "bne";
        printf("%s  $%d, $%d, %d\n", instruction, s, t, I);
    } else if (which_instruction == ADDI) {
         instruction = "addi";
         printf("%s $%d, $%d, %d\n", instruction, t, s, I);
    } else if (which_instruction == SLTI) {
         instruction = "slti";
         printf("%s  $%d, $%d, %d\n", instruction, t, s, I);
    } else if (which_instruction == ANDI) {
         instruction = "andi";
         printf("%s  $%d, $%d, %d\n", instruction, t, s, I);
    } else if (which_instruction == ORI) {
         instruction = "ori";
         printf("%s  $%d, $%d, %d\n", instruction, t, s, I);
    } else {
        instruction = "lui";
        printf("%s  $%d, %d\n", instruction, t, I);        
    }    
    return;
}

// Reads in 32 bit instruction and runs the instruction
void run_instruction_set1 (int bit_instruction, int *pc, int *registers){

    // Extract value of registers from bit pattern
    int s = decode_registers_s (bit_instruction);
    int t = decode_registers_t (bit_instruction);
    int d = decode_registers_d (bit_instruction);
    
    // Extract last 6 bits from bit pattern (the instruction)
    uint32_t instruction = extract_bit_range (bit_instruction, 5, 0);
    
    // Identify MIPS instruction and run instruction
    
    if (instruction == SYSCALL) {
        // if $v0 = 1, print $a0 as integar
        if (registers[2] == 1) {
            printf("%d", registers[4]);        
        }
        // if $v0 = 11, print $a0 as character
        else if (registers[2] == 11) {
            printf("%c", registers[4]);   
           // if $v0 = 11, exit output loop in main            
        }  else if (registers[2] == 10) {
            *pc = -1;          //set pc to -1 to end loop
            return;
          // Invalid syscall, print error message and exit output loop
        } else {
            printf("Unknown system call: %d\n",registers[2]);
            *pc = -1;          //set pc to -1 to end loop
            return;
        }
    }
    // If register value is stored in is $0, skip running instruction
    if (d == 0) {
        *pc = *pc + 1;
        return;
    }
    if (instruction == ADD) {
        registers[d] = registers[s] + registers[t];
    }
    if (instruction == SUB) {
        registers[d] = registers[s] - registers[t];
    }
    if (instruction == AND) {
        registers[d] = registers[s] && registers[t];
    }
    if (instruction == OR) {
        registers[d] = registers[s] || registers[t];
    }
    if (instruction == SLT) {
        if (registers[s] < registers[t]) {
            registers[d] = 1;
        } else {
            registers[d] = 0;
        }
    }
    // Move program_counter to read next instruction
    *pc = *pc + 1;
    return;
}

// Reads in 32 bit instruction and runs the instruction
void run_instruction_set2 (int bit_instruction, int *pc, int *registers) {

    // Extract value of registers and immediate value from bit pattern
    int s = decode_registers_s (bit_instruction);
    int t = decode_registers_t (bit_instruction);
    short I = decode_immediate (bit_instruction);
    int d = decode_registers_d (bit_instruction);
    
    // Extract first 6 bits from bit pattern (the instruction)
    uint32_t instruction = extract_bit_range (bit_instruction, INSTRUCTION_HIGH_BIT, INSTRUCTION_LOW_BIT);
    
    // Identify MIPS instruction and run instruction
    if (instruction == MUL) {
        // If register value is stored in is $0, skip running instruction        
        if (d == 0) {
        *pc = *pc + 1;
            return;
        } else {
            registers[d] = registers[s] * registers[t];
        }
    }
    if (instruction == BEQ) {
        if (registers[s] == registers[t]) {
            // Iterate program counter by immediate value
            *pc += I;
            return;
        }    
    }
    if (instruction == BNE) {
        if (registers[s] != registers[t]) {
            // Iterate program counter by immediate value
            *pc += I;
            return;
        }    
    }
    if (instruction == ADDI) {
        registers[t] = registers[s] + I;
    }
    if (instruction == SLTI) {
        registers[t] = registers[s] << I;
    }
    if (instruction == ANDI) {
        registers[t] = registers[s] & I;
    }
    if (instruction == ORI) {
        registers[t] = registers[s] | I;
    }
    if (instruction == LUI) {
        registers[t] = I << 16;
    }    
    // Move program_counter to read next instruction
    *pc = *pc + 1;
    return;
 }

// Extracts value of the registers from instruction bits

int decode_registers_d (int bit_instruction){
    int d = extract_bit_range(bit_instruction,registers_D_HIGH_BIT,registers_D_LOW_BIT);
    return d;
}
int decode_registers_s (int bit_instruction){
    int s = extract_bit_range(bit_instruction,registers_S_HIGH_BIT,registers_S_LOW_BIT);
    return s;
}
int decode_registers_t (int bit_instruction){
    int t = extract_bit_range(bit_instruction,registers_T_HIGH_BIT,registers_T_LOW_BIT);
    return t;
}
short decode_immediate (int bit_instruction){
    short I = extract_bit_range(bit_instruction,IMMEDIATE_HIGH_BIT, IMMEDIATE_LOW_BIT);
    return I;
}

// Loop through and print registers that have non-zero value
void print_registers (int *registers) {

    for (int i = 0; i < 32; i++) {
        if (registers[i] != 0) {
            printf("$%-2d = %d\n", i, registers[i]);
        }
    }
}

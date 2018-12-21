// SEGGER Embedded Studio, runtime support.
//
// Copyright (c) 2014-2017 SEGGER Microcontroller GmbH & Co KG
// Copyright (c) 2001-2017 Rowley Associates Limited.
//
// This file may be distributed under the terms of the License Agreement
// provided with this software.
//
// THIS FILE IS PROVIDED AS IS WITH NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
//
//                           Preprocessor Definitions
//                           ------------------------
// APP_ENTRY_POINT
//
//   Defines the application entry point function, if undefined this setting
//   defaults to "main".
//
// INITIALIZE_STACK
//
//   If defined, the contents of the stack will be initialized to a the
//   value 0xCC.
//
// INITIALIZE_SECONDARY_SECTIONS
//
//   If defined, the .data2, .text2, .rodata2 and .bss2 sections will be initialized.
//
// INITIALIZE_TCM_SECTIONS
//
//   If defined, the .data_tcm, .text_tcm, .rodata_tcm and .bss_tcm sections 
//   will be initialized.
//
// INITIALIZE_USER_SECTIONS
//
//   If defined, the function InitializeUserMemorySections will be called prior
//   to entering main in order to allow the user to initialize any user defined
//   memory sections.
//
// FULL_LIBRARY
//
//  If defined then 
//    - argc, argv are setup by the debug_getargs.
//    - the exit symbol is defined and executes on return from main.
//    - the exit symbol calls destructors, atexit functions and then debug_exit.
//  
//  If not defined then
//    - argc and argv are zero.
//    - the exit symbol is defined, executes on return from main and loops
//

#ifndef APP_ENTRY_POINT
#define APP_ENTRY_POINT main
#endif

  .syntax unified

  .global _start
  .extern APP_ENTRY_POINT
  .section .init, "ax"
  .code 16
  .align 2
  .thumb_func

_start:
  /* Copy initialized memory sections into RAM (if necessary).*/ 
  ldr r0, =__data_load_start__
  ldr r1, =__data_start__
  ldr r2, =__data_end__
  bl memory_copy


  /* Zero the bss. */
  ldr r0, =__bss_start__
  ldr r1, =__bss_end__
  movs r2, #0
  bl memory_set

  .type start, function
start:
  /* Jump to application entry point */
  movs r0, #0
  movs r1, #0
  ldr r2, =APP_ENTRY_POINT
  blx r2




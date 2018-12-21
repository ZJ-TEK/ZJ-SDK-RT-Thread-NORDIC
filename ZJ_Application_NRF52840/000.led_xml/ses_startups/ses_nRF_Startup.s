/*****************************************************************************
 *                   SEGGER Microcontroller GmbH & Co. KG                    *
 *            Solutions for real time microcontroller applications           *
 *****************************************************************************
 *                                                                           *
 *               (c) 2017 SEGGER Microcontroller GmbH & Co. KG               *
 *                                                                           *
 *           Internet: www.segger.com   Support: support@segger.com          *
 *                                                                           *
 *****************************************************************************/

/*****************************************************************************
 *                         Preprocessor Definitions                          *
 *                         ------------------------                          *
 * NO_FPU_ENABLE                                                             *
 *                                                                           *
 *   If defined, FPU will not be enabled.                                    *
 *                                                                           *
 * NO_STACK_INIT                                                             *
 *                                                                           *
 *   If defined, the stack pointer will not be initialised.                  *
 *                                                                           *
 * NO_SYSTEM_INIT                                                            *
 *                                                                           *
 *   If defined, the SystemInit() function will not be called. By default    *
 *   SystemInit() is called after reset to enable the clocks and memories to *
 *   be initialised prior to any C startup initialisation.                   *
 *                                                                           *
 * NO_VTOR_CONFIG                                                            *
 *                                                                           *
 *   If defined, the vector table offset register will not be configured.    *
 *                                                                           *
 * MEMORY_INIT                                                               *
 *                                                                           *
 *   If defined, the MemoryInit() function will be called. By default        *
 *   MemoryInit() is called after SystemInit() to enable an external memory  *
 *   controller.                                                             *
 *                                                                           *
 * STACK_INIT_VAL                                                            *
 *                                                                           *
 *   If defined, specifies the initial stack pointer value. If undefined,    *
 *   the stack pointer will be initialised to point to the end of the        *
 *   RAM segment.                                                            *
 *                                                                           *
 * VECTORS_IN_RAM                                                            *
 *                                                                           *
 *   If defined, the exception vectors will be copied from Flash to RAM.     *
 *                                                                           *
 *****************************************************************************/
.syntax unified
  .global Reset_Handler
  .section .init, "ax"
  .thumb_func
Reset_Handler:
  /* Jump to program start */
  b _start


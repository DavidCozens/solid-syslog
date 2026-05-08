/* Empty stub of FreeRTOS-Plus-TCP's compiler-portable struct-packing helper.
 *
 * Real Plus-TCP installs ship per-compiler variants under
 * source/portable/Compiler/<GCC|IAR|MSVC|...>/. Tests must stay independent
 * of which toolchain the integrator's production build will use, so
 * FreeRtosFakes provides this empty stub instead of pulling one specific
 * compiler's directory into the test include path.
 *
 * Tests never run the IP stack, so packed-struct layout is irrelevant; the
 * stub does nothing.
 */

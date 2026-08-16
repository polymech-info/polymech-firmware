*-----------------------------------------------------------------------------
*  Macro Library
*-----------------------------------------------------------------------------
*  Macro's for error Messages
*
*-----------------------------------------------------------------------------

#define /m    M_error_display(No)        IFT                              \
  PS     (No)                                                             \
  CM     9085                                                             \
  L      NP_W1022_Modul_Fehlerstatus                                      \
  =      WG_error_status                                                  \
ENDI

#define /m    M_display_module_error(No) L  NP_M4203_error_module_9xxx     \
IFT                                                                        \
  PS     (No)                                                              \
  CM     9085                                                              \
  L      NP_W1022_Modul_Fehlerstatus                                       \
  =      WG_error_status                                                   \
ENDI

#define /m    M_module_error_display(No,M_win) L  NP_M4203_error_module_9xxx     \
IFT                                                                              \
  PS     (No)                                                                    \
  CM     9085                                                                    \
  L      MG_marker_one                                                           \
  S      (M_win)                                                                 \
  L      NP_W1022_Modul_Fehlerstatus                                             \
  =      WG_error_status                                                         \
ENDI


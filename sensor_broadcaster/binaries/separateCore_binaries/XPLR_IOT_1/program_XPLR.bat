nrfjprog --recover
nrfjprog -f NRF53 --coprocessor CP_NETWORK --program net_core_XPLR.hex --chiperase
nrfjprog -f NRF53 --program app_core_XPLR.hex --chiperase
nrfjprog --pinreset

pause
# Sensor Bluetooth LE Broadcaster Binaries

This folder contains the compiled binary images for Flashing an XPLR-IOT-1 or MINI-NORA-B1 Evaluation Kit.

- Files names ending in `MINI` refer to MINI-NORA-B1 Evaluation Kit.
- Files names ending in `XPLR` refer to XPLR-IOT-1.

All binaries in this folder require to connect a **J-Link** to your board to Flash the image.

#### Merged Core Binaries

The [mergedCore_binaries folder](./mergedCores_binaries/) contains binaries that can be flashed using the [nRF Connect](https://www.nordicsemi.com/Products/Development-tools/nrf-connect-for-desktop) -> Programmer application. Those binaries contain the merged image for both NETWORK and APPLICATION cores.

#### Separate Core Binaries

The [separateCore_binaries](./separateCore_binaries/) folder contains the binaries for NETWORK and APPLICATION core separate (in case you want to use nrftools or something else to Flash your boards). 

Batch scripts are also contained to help you Flash the image. Just connect a J-Link to your board and run the batch script ([nRF Command Line Tools](https://www.nordicsemi.com/Products/Development-tools/nrf-command-line-tools) should be installed and `nrfjprog`command should be available to your command line)


## Disclaimer
Copyright &copy; u-blox 

u-blox reserves all rights in this deliverable (documentation, software, etc.,
hereafter “Deliverable”). 

u-blox grants you the right to use, copy, modify and distribute the
Deliverable provided hereunder for any purpose without fee.

THIS DELIVERABLE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
WARRANTY. IN PARTICULAR, NEITHER THE AUTHOR NOR U-BLOX MAKES ANY
REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY OF THIS
DELIVERABLE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.

In case you provide us a feedback or make a contribution in the form of a
further development of the Deliverable (“Contribution”), u-blox will have the
same rights as granted to you, namely to use, copy, modify and distribute the
Contribution provided to us for any purpose without fee.
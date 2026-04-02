# Amstrad-CPC-IO-Logger

>[!CAUTION]
>USE AT YOUR OWN RISK.
>There is always the risk that this Amstrad CPC IO Logger could cause harm to your CPC. Although I have tested the Amstrad CPC IO Logger, there is no guarantee that it will work properly under all circumstances.

>[!WARNING]
>This project is still in development and has only been made public to share the ideas and solicit feedback. I am no expert in this area and the project should not be used as a baseline example but more an idea of what can be done with the Pico.

## Outline
This Amstrad CPC IO Logger is an external device which allows values written to the Gate Array and the CRTC to be retrieved. This can be useful, for example, for a debugger that exposes the state of these chips in a dedicated UI section.

Port &7F is used by the Gate Array and is write-only. On a standard Amstrad CPC, there is no ability to read back the values sent to the Gate Array (video mode, pen colours, ROM/RAM configuration, etc.). The firmware keeps a shadow copy of all written values in RAM for its own internal use, but many programs bypass the firmware entirely for such operations.

Port &BF can be used to read values from the CRTC, but only on certain CRTC types, and even then, only a subset of registers is readable. The firmware only maintains a shadow copy of R12–R13 (display start address). The remaining registers cannot be read back reliably across all machines, as behaviour varies depending on the CRTC type installed.

## Summary
The Amstrad CPC only uses the top 8 address lines, A8-A15, to address standard IO devices, as illustrated below. For more information please refer to the [CPC Wiki Default IO Port Summary page](https://www.cpcwiki.eu/index.php/Default_I/O_Port_Summary). To see a list of all known ports please refer to the [CPC Wiki IO Port Summary page](https://www.cpcwiki.eu/index.php/I/O_Port_Summary). This Amstrad CPC IO logger then captures A8-A15 and D0-D7 associated with any IO Write that occurs (i.e. a Z80 OUT instruction). Any IO Writes associated with the Gate Array or CRTC are captured and can be retrieved at any point with a series of simple IO Reads to the Amstrad CPC IO logger (i.e. a Z80 IN instruction).

<img width="1141" height="583" alt="Amstrad CPC IO Addressing" src="https://github.com/user-attachments/assets/00889eaa-376c-46f6-a24c-d59acc8cbe7f" />

## Design Summary
The discrete 74 series logic associated with Amstrad CPC IO Logger PCB presents two signals* to the Pico;
- IOWR: Active low on an IO Write instruction from the Amstrad CPC
- CSRD: Active low when on an IO read instruction from the Amstrad CPC for the Amstrad CPC IO Logger IO Address (currently F9E0).

<img width="1087" height="558" alt="Amstrad CPC IO Logger Design" src="https://github.com/user-attachments/assets/0eea43fd-a71e-4c0d-b5b7-e629a37eff75" />


### Capture
When IOWR goes low, State Machine 1 (SM1) will push the contents of A8-A15** and D0-D7 onto its RX FIFO. DMA1 detects this and places the data into a Capture Buffer. Note that the Capture buffer is configured as a Rig Buffer.

**A0-A1 are also capture but are not used.

The contents of the Capture Buffer is decoded and the CRTC and Gate Array Register write values are placed into a Register Buffer.

### Retrieve
When CSRD goes low after an IN instruction (i.e. after INP(&F9E0) from BASIC), SM2 places a transfer count of 1 on it's RX FIFO. DMA2 detects this and places this value into the Transfer Count Trigger of DMA3. DMA3 detects this and then places the next register value from the Register Buffer onto the TX FIFO of SM2. This is then returned back to the Amstrad CPC. Repeated IN instructions will return the next value in the Register Buffer.

### Register Buffer
The Register Buffer is a sequential list of register values written to the CRTC and Gate Array. Each IN instruction to the Amstrad CPC IO Logger will return the next value in the Register Buffer. An OUT instruction of 1 (i.e. OUT &F9E0,1 from BASIC) will reset the sequence back to the first value in the Register Buffer (i.e. CRTC R0).

<img width="1633" height="138" alt="Register Buffer" src="https://github.com/user-attachments/assets/7357d135-0b1c-4327-a24f-545cfb51d31e" />

- LR is the last CRTC Register written to.
- LP is the last GA Pen written to.
- L16 is the Border Pen

Please refer to the [CPC Wiki CRTC page](https://www.cpcwiki.eu/index.php/CRTC) and [CPC Wiki Gate Array page](https://www.cpcwiki.eu/index.php/Gate_Array) for more information.

## Acknowledgements
- **NoRecess** for the inspiration and support on this project.
- **RHD** members for support and encouragement.
- **Petersfield Men's Shed IT and Electronics Group** members for support and encouragement.

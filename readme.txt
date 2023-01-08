BOOM Parameters

[C0] ======BOOM Tile 0 Params======

[C0] ====Overall Frontend Params====

[C0] ==L1-ICache==
[C0] Fetch bytes   : 8
[C0] Block bytes   : 64
[C0] Row bytes     : 8
[C0] Word bits     : 64
[C0] Sets          : 64
[C0] Ways          : 4
[C0] Refill cycles : 8
[C0] RAMs          : (64 x 512) using 1 banks
[C0] Single-banked
[C0] I-TLB ways    : 32

[C0] ==Branch Predictor Memory Sizes==

[C0] bank0 tage_l2: 128 x 44 = 704
[C0] bank0 tage_l4: 128 x 44 = 704
[C0] bank0 tage_l8: 256 x 48 = 1536
[C0] bank0 tage_l16: 256 x 48 = 1536
[C0] bank0 tage_l32: 128 x 52 = 832
[C0] bank0 tage_l64: 128 x 52 = 832
[C0] bank0 btb_meta_way0: 128 x 124 = 1984
[C0] bank0 btb_data_way0: 128 x 56 = 896
[C0] bank0 btb_meta_way1: 128 x 124 = 1984
[C0] bank0 btb_data_way1: 128 x 56 = 896
[C0] bank0 ebtb: 128 x 40 = 640
[C0] bank0 bim: 2048 x 8 = 2048
[C0] Total bpd size: 14 KB

[C0] ====Overall Core Params====

[C0] ===ExecutionUnits===

[C0] ==1-wide Machine==
[C0] ==3 Issue==

[C0] ==ExeUnit==
[C0]  - Mem
[C0] ==ExeUnit==
[C0]  - ALU
[C0]  - Mul
[C0]  - Div
[C0]  - IFPU

[C0] ===FP Pipeline===

[C0] ==Floating Point Regfile==
[C0] Num RF Read Ports     : 3
[C0] Num RF Write Ports    : 2
[C0] RF Cost (R+W)*(R+2W)  : 35
[C0] Bypassable Units      : List(false, false)
[C0] Num Wakeup Ports      : 2
[C0] Num Bypass Ports      : 0

[C0] ==ROB==
[C0] Machine Width      : 1
[C0] Rob Entries        : 32
[C0] Rob Rows           : 32
[C0] Rob Row size       : 5
[C0] log2Ceil(coreWidth): 0
[C0] FPU FFlag Ports    : 2

[C0] ===Other Core Params===
[C0] Fetch Width           : 4
[C0] Decode Width          : 1
[C0] Issue Width           : 3
[C0] ROB Size              : 32
[C0] Issue Window Size     : List(8, 8, 8) (Age-based Priority)
[C0] Load/Store Unit Size  : 8/8
[C0] Num Int Phys Registers: 52
[C0] Num FP  Phys Registers: 48
[C0] Max Branch Count      : 8
[C0] ==Integer Regfile==
[C0] Num RF Read Ports     : 4
[C0] Num RF Write Ports    : 2
[C0] RF Cost (R+W)*(R+2W)  : 48
[C0] Bypassable Units      : List(true, true)

[C0] Num Slow Wakeup Ports : 2
[C0] Num Fast Wakeup Ports : 1
[C0] Num Bypass Ports      : 3

[C0] DCache Ways           : 4
[C0] DCache Sets           : 64
[C0] DCache nMSHRs         : 2
[C0] ICache Ways           : 4
[C0] ICache Sets           : 64
[C0] D-TLB Ways            : 8
[C0] I-TLB Ways            : 32
[C0] Paddr Bits            : 32
[C0] Vaddr Bits            : 39

[C0] Using FPU Unit?       : true
[C0] Using FDivSqrt?       : true
[C0] Using VM?             : true


Interrupt map (2 harts 8 interrupts):
  [1, 8] => gen

/ {
        #address-cells = <1>;
        #size-cells = <1>;
        compatible = "freechips,rocketchip-vivado-dev";
        model = "freechips,rocketchip-vivado";
        L14: cpus {
                #address-cells = <1>;
                #size-cells = <0>;
                timebase-frequency = <1000000>;
                L3: cpu@0 {
                        clock-frequency = <0>;
                        compatible = "ucb-bar,boom0", "riscv";
                        d-cache-block-size = <64>;
                        d-cache-sets = <64>;
                        d-cache-size = <16384>;
                        d-tlb-sets = <1>;
                        d-tlb-size = <8>;
                        device_type = "cpu";
                        hardware-exec-breakpoint-count = <0>;
                        i-cache-block-size = <64>;
                        i-cache-sets = <64>;
                        i-cache-size = <16384>;
                        i-tlb-sets = <1>;
                        i-tlb-size = <32>;
                        mmu-type = "riscv,sv39";
                        next-level-cache = <&L9>;
                        reg = <0x0>;
                        riscv,isa = "rv64imafdc";
                        riscv,pmpgranularity = <4>;
                        riscv,pmpregions = <8>;
                        status = "okay";
                        timebase-frequency = <1000000>;
                        tlb-split;
                        L2: interrupt-controller {
                                #interrupt-cells = <1>;
                                compatible = "riscv,cpu-intc";
                                interrupt-controller;
                        };
                };
        };
        L9: memory@80000000 {
                device_type = "memory";
                reg = <0x80000000 0x80000000>;
        };
        L13: soc {
                #address-cells = <1>;
                #size-cells = <1>;
                compatible = "freechips,rocketchip-vivado-soc", "simple-bus";
                ranges;
                L5: clint@2000000 {
                        compatible = "riscv,clint0";
                        interrupts-extended = <&L2 3 &L2 7>;
                        reg = <0x2000000 0x10000>;
                        reg-names = "control";
                };
                L6: debug-controller@0 {
                        compatible = "sifive,debug-013", "riscv,debug-013";
                        debug-attach = "dmi";
                        interrupts-extended = <&L2 65535>;
                        reg = <0x0 0x1000>;
                        reg-names = "control";
                };
                L1: error-device@3000 {
                        compatible = "sifive,error0";
                        reg = <0x3000 0x1000>;
                };
                L8: external-interrupts {
                        interrupt-parent = <&L4>;
                        interrupts = <1 2 3 4 5 6 7 8>;
                };
                L4: interrupt-controller@c000000 {
                        #interrupt-cells = <1>;
                        compatible = "riscv,plic0";
                        interrupt-controller;
                        interrupts-extended = <&L2 11 &L2 9>;
                        reg = <0xc000000 0x4000000>;
                        reg-names = "control";
                        riscv,max-priority = <7>;
                        riscv,ndev = <8>;
                };
                L10: mmio-port-axi4@60000000 {
                        #address-cells = <1>;
                        #size-cells = <1>;
                        compatible = "simple-bus";
                        ranges = <0x60000000 0x60000000 0x20000000>;
                };
                L11: rom@10000 {
                        compatible = "sifive,rom0";
                        reg = <0x10000 0x10000>;
                        reg-names = "mem";
                };
                L0: subsystem_pbus_clock {
                        #clock-cells = <0>;
                        clock-frequency = <100000000>;
                        clock-output-names = "subsystem_pbus_clock";
                        compatible = "fixed-clock";
                };
        };
};

Generated Address Map
               0 -     1000 ARWX  debug-controller@0
            3000 -     4000 ARWX  error-device@3000
           10000 -    20000  R X  rom@10000
         2000000 -  2010000 ARW   clint@2000000
         c000000 - 10000000 ARW   interrupt-controller@c000000
        60000000 - 80000000  RWX  mmio-port-axi4@60000000
        80000000 - 100000000  RWXC memory@80000000
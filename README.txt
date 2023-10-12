1. mysbi:
When the cpu is reset, it is in M mode. Then the program starts from the first instruction in the mysbi/start.S.Mysbi run in M mode，it provides entry of program, exception process and sbi runtime interface.It enters into S mode in function sbi_jump_to_supervisor at the end of sbi_init.

(1) entry point
The entry point of mysbi is _start which is defined in mysbi/entry/start.S. It set sp to stack top, then call sbi_init if the hartid is 0 (boot hart), while other hart while spin here. 
(2) exception process
The entry point of exception is exception_vector which is defined in mysbi/sbi/sbi_entry.S. It save the context in stack, and then call sbi_trap_handler to process exception, interrupt and sbi call.  
(3) device table
The hardware info is defined in bsp/hw.c. Mysbi read this info and pass its address into gos.

2. gos:
We set the address located in gos as the target address of mret. When the program execute mret in sbi_jump_to_supervisor in mysbi, cpu will switch to S mode and the pc while be set to the address in start of gos/entry/start.S what defined in gos.lds. In gos, we provide 
a、A simple memory management based on bitmap.
b. Exeception process for peripheral interrupt and timer interrupt(clint).
c. A system tick and a simple system timer.
d. A simple interrupt process framework.
e. A simple device driver framework.
f. A simple shell used to execute command.
g. A simple framework to add a command and execute command in the shell or auto run the specific commands which are defined in auto_run.bin in order.

3. Memroy map
+-------+ 0x80000000 (mysbi) (start of ddr)
+ text  +
+-------+
+ data  +
+-------+
+ rodat +
+-------+
+  bss  +
+-------+
+ device+
+ table +
+-------+
 ......
+-------+ 0x80010000 (gos)
+ text  +
+-------+
+ data  +
+-------+
+ rodat +
+-------+
+ driver+
+ table +
+-------+
+ early +
+ con   +
+ table +
+-------+
+irqchip+
+ table +
+-------+
+ timer +
+ table +
+-------+
+command+
+ table +
+-------+
+  bss  +
+-------+ start of free memory
+       +
+ free  +
+memory +
+       +
+       +
+-------+ end of free memory(end of ddr)

4. device and driver probe
In _start -> start_gos -> device_driver_init -> __probe_device_table, it probe the device_table passed from mysbi and mach each item with the drivers defined in driver table through compatible attribute, if success, the init funciton of matched driver while be called. 
When you want to add a driver into gos, use DRIVER_REGISTER(name, init_fn, compat) to define a driver. Name is the driver's name, init_fn is the entry point of the driver and compat is used to match witch device. If it is matched successful, the init_fn will be called.

5. Shell
When gos initialization is completed, it will go into the shell. You can enter the name of the cmd, then the cmd is executed.
Now, We provide some simple command, as following:
a. hello -- print hello info...
b. ls -- print all commands present in gos.
c. devices -- print hardware info defined in bsp/hw.c.
d. devmem -- read or write mmio.
e. history -- print executed history command.

How to add a command:
a. Create a xxx.c file in app/command/ and add "obj-y += xxx.o" in app/command/Makefile.
b. In xxx.c file, use APP_COMMAND_REGISTER(name, init_fn) to define a entry function.
c. In init_fn entry function, call register_command to register a command, it need a static variable of struct command where the cmd is the command name which is entered in shell, handler is the callback which is called when cmd is entered in shell, priv is a void* pointer passed into the handler as the last param.
d. Define the handler to do what you want to do.
e. The handler is defined as:
	int (*handler)(int argc, char *argv[], void *priv);
   argc -- Number of params entered in the shell behind the comand.
   argv -- Params entered in the shell.
   priv -- A pointer defined in the struct command.

auto run:
When gos initialization is completed, it will print a 5s countdown, if ESC is entered during this 5s, it will enter into the shell. Otherwise, it will run the command defined in auto_run.bin in order.

6. Add a new peripheral
a. Add the hardware info in bsp/hw.c include compatible used to match drivers in gos, start(the physical address), len, irq(hwirq number), and data(user defined info, NULL if not neccessary).
b. Add a c file in drivers/ and add "obj-y += xxx.o" in Makefile.
c. In c file, use DRIVER_REGISTER to register a driver.

ps. Considering the decoupling of the program. It is best to follow the calling process of app -> hal -> drivers.

7. Usage
make -- complier the project with -g -O0. The output files is out/Image.bin which is packed from mysbi.bin and gos.bin. The mysbi.bin and gos.bin is in build/, meanwhile, the mysbi.elf/gos.elf and mysbi.map/gos.map is also in the build/.

make clean -- clean all

make run -- Run in qemu(This need to open DEBUG marco in Makefile).

make run-debug -- Run in qemu -S -s.

make format -- format the c files and h files use Lindent script.

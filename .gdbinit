target remote localhost:1234
symbol-file kernel/kernel.sym
define hook-quit
	set confirm off
  end
define p
  print/x $arg0
  end
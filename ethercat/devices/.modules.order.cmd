cmd_/home/fudanrobotuser/ethercat/devices/modules.order := {   echo /home/fudanrobotuser/ethercat/devices/ec_generic.ko; :; } | awk '!x[$$0]++' - > /home/fudanrobotuser/ethercat/devices/modules.order

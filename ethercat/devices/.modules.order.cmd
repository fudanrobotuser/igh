cmd_/home/fudanrobotuser/github/igh/ethercat/devices/modules.order := {   echo /home/fudanrobotuser/github/igh/ethercat/devices/ec_generic.ko; :; } | awk '!x[$$0]++' - > /home/fudanrobotuser/github/igh/ethercat/devices/modules.order

cmd_/home/fudanrobotuser/ethercat/master/modules.order := {   echo /home/fudanrobotuser/ethercat/master/ec_master.ko; :; } | awk '!x[$$0]++' - > /home/fudanrobotuser/ethercat/master/modules.order

## This code is an example of the program used to control
## the time of an experiment.

FIFO=/tmp/tempFIFO

SAMPLE="Cs-137"
TIME="30 mins"
TRIGGER="0.20V"
SOFTWARETRIGGER="0.225V"
VREF="2.7V"
VMAX="1.0V"
GAIN="1"

function echoTime {
	date +"[%c]: $1"
}

## This is the most important function as all it does is
## send the command to the FIFO which the program picks up
## and then sends to the microprocessor. It also displays it
function send {
	echoTime $1
	echo $1 > $FIFO
	sleep 1
}

echoTime "Starting experiment with sample $SAMPLE and with a duration of $TIME"
echoTime "With a hardware trigger of $TRIGGER, and a software trigger of $SOFTWARETRIGGER"
echoTime "Finally, we use a Vref of $VREF, maximum voltage of $VMAX, and a gain of $GAIN"
send "resetMCAValues"
send "setFilterVoltage 100" #not in millivolts/volts/etc
send "startMCA"

sleep 30m

send "sendMCAValues"
send "stopMCA"
send "resetMCAValues"
echoTime "Ending experiment with sample $SAMPLE"
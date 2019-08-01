ffrMake: forwardFacingRadar.c fileManagement.c socketCommunication.c
	gcc forwardFacingRadar.c fileManagement.c socketCommunication.c -o ffrMaked
	clear
	@echo "ffr è stato creato correttamente."
fwcMake: frontWindshieldCamera.c fileManagement.c socketCommunication.c
	gcc frontWindshieldCamera.c fileManagement.c socketCommunication.c -o fwc
	clear
	@echo "fwc è stato creato correttamente."
bsMake: blindspot.c fileManagement.c socketCommunication.c
	gcc blindspot.c fileManagement.c socketCommunication.c -o bs -I
	clear
	@echo "bs è stato creato correttamente."
svcMake: surroundViewCameras.c fileManagement.c socketCommunication.c
	gcc surroundViewCameras.c fileManagement.c socketCommunication.c -o svc
	clear
	@echo "svc è stato creato correttamente."
paMake: parkassist.c fileManagement.c socketCommunication.c
	gcc parkassist.c fileManagement.c socketCommunication.c -o pa
	clear
	@echo "pa è stato creato correttamente."
sensorMake: 
	gcc forwardFacingRadar.c fileManagement.c socketCommunication.c -o ffrMaked
	gcc frontWindshieldCamera.c fileManagement.c socketCommunication.c -o fwcMaked
	gcc blindSpot.c fileManagement.c socketCommunication.c -o bsMaked
	gcc surroundViewCameras.c fileManagement.c socketCommunication.c -o svcMaked
	gcc parkAssist.c fileManagement.c socketCommunication.c -o paMaked
	clear
	@echo "Tutti i sensori sono stati creati correttamente."
	@echo "I file creati sono: ffrMaked, fwcMaked, bsMaked, svcMaked, paMaked."
allMake:
	gcc frontWindshieldCamera.c fileManagement.c socketCommunication.c -o fwc
	gcc forwardFacingRadar.c fileManagement.c socketCommunication.c -o ffr
	gcc blindSpot.c fileManagement.c socketCommunication.c -o bs
	gcc surroundViewCameras.c fileManagement.c socketCommunication.c -o svc
	gcc parkAssist.c fileManagement.c socketCommunication.c -o pa
	gcc steerByWire.c fileManagement.c socketCommunication.c -o sbw
	gcc brakeByWire.c fileManagement.c socketCommunication.c -o bbw
	gcc throttleControl.c fileManagement.c socketCommunication.c -o tc
	gcc ECU.c fileManagement.c socketCommunication.c byteComparison.c -o ecu	
	gcc hmi.c socketCommunication.c fileManagement.c -o hmi
    

	
	

simple RTPApplication

	parameters:

		commonName: string,
		profileName: string,
		bandwidth,
		destinationAddress: string,
		portNumber,
		fileName: string,
		payloadType,
		sessionEnterDelay,
		transmissionStartDelay,
		transmissionStopDelay,
		sessionLeaveDelay;

	gates:
		out: toRTP;
		in: fromRTP;


endsimple

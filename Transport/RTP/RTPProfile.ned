simple RTPProfile

	parameters:
		autoOutputFileNames: string;

	gates:
		in: fromRTP, fromPayloadSender, fromPayloadReceiver[];
		out: toRTP, toPayloadSender, toPayloadReceiver[];

endsimple
#include "stdafx.h"

int main(int argc, char *argv[])
{
	char *hostname;

	if (argc > 1) {
		hostname = argv[1];
	} else {
		exit(1);
	}

	WINSANE_Session *session = WINSANE_Session::Remote(hostname);

	if (session->Init(NULL, NULL)) {
		int devices = session->GetDevices();

		for (int i = 0; i < devices; i++) {
			WINSANE_Device *device = session->GetDevice(i);

			if (device->Open()) {
				printf("------------------------------------------------------------\n");
				printf("Name:\t%s\n", device->GetName());
				printf("Vendor:\t%s\n", device->GetVendor());
				printf("Model:\t%s\n", device->GetModel());
				printf("Type:\t%s\n", device->GetType());

				int options = device->GetOptions();

				for (int j = 0; j < options; j++) {
					WINSANE_Option *option = device->GetOption(j);

					printf("------------------------------------------------------------\n");
					printf("Name:\t\t%s\n", option->GetName());
					printf("Title:\t\t%s\n", option->GetTitle());
					printf("Description:\t%s\n", option->GetDescription());
				}

				WINSANE_Params *params = device->GetParams();

				printf("------------------------------------------------------------\n");
				printf("Format:\t\t%d\n", params->GetFormat());
				printf("Last Frame:\t%d\n", params->IsLastFrame());
				printf("Bytes per Line:\t%d\n", params->GetBytesPerLine());
				printf("Pixel per Line:\t%d\n", params->GetPixelsPerLine());
				printf("Lines:\t\t%d\n", params->GetLines());
				printf("Depth:\t\t%d\n", params->GetDepth());

				if (i == 1) {
					WINSANE_Scan *scan = device->Start();
					if (scan) {
						Sleep(5000);

						device->Cancel();
					}
				}

				device->Close();
			}
		}

		session->Exit();

		Sleep(5000);
	}

	delete session;

	exit(0);
}

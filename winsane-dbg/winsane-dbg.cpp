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

				int options = device->FetchOptions();

				for (int j = 0; j < options; j++) {
					WINSANE_Option *option = device->GetOption(j);

					printf("------------------------------------------------------------\n");
					printf("Name:\t\t%s\n", option->GetName());
					printf("Title:\t\t%s\n", option->GetTitle());
					printf("Description:\t%s\n", option->GetDescription());

					switch (option->GetType()) {
						case SANE_TYPE_BOOL:
							printf("Value:\t\t%d\n", option->GetValueBool());
							break;
						case SANE_TYPE_INT:
							printf("Value:\t\t%d\n", option->GetValueInt());
							break;
						case SANE_TYPE_FIXED:
							printf("Value:\t\t%d\n", option->GetValueFixed());
							break;
						case SANE_TYPE_STRING:
							SANE_String value = option->GetValueString();
							if (value) {
								printf("Value:\t\t%s\n", value);
								delete value;
							} else {
								printf("Value:\t\t<NULL>\n");
							}
							break;
					}
				}

				WINSANE_Params *params = device->GetParams();

				printf("------------------------------------------------------------\n");
				printf("Format:\t\t%d\n", params->GetFormat());
				printf("Last Frame:\t%d\n", params->IsLastFrame());
				printf("Bytes per Line:\t%d\n", params->GetBytesPerLine());
				printf("Pixel per Line:\t%d\n", params->GetPixelsPerLine());
				printf("Lines:\t\t%d\n", params->GetLines());
				printf("Depth:\t\t%d\n", params->GetDepth());

				delete params;

				device->Cancel();

				WINSANE_Scan *scan = device->Start();
				if (scan) {
					printf("Begin scanning image ...\n");
					HANDLE output = CreateFile(L"winsane-dbg.scan", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
					DWORD written;

					char *buffer = new char[1024];
					long length = 1024;
					while (scan->AquireImage(buffer, &length) == CONTINUE) {
						printf("Received %d bytes of scanned image ...\n", length);
						WriteFile(output, buffer, length, &written, NULL);
						length = 1024;
					}
					CloseHandle(output);
					printf("Finished scanning image!\n");

					delete scan;

					device->Cancel();
				}

				device->Close();
			}
		}

		session->Exit();
	}

	delete session;

	exit(0);
}

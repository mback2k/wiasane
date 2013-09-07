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
							printf("Value:\t\t%f\n", SANE_UNFIX(option->GetValueFixed()));
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

					switch (option->GetUnit()) {
						case SANE_UNIT_NONE:
							printf("Unit:\t\tNone\n");
							break;
						case SANE_UNIT_PIXEL:
							printf("Unit:\t\tPixel\n");
							break;
						case SANE_UNIT_BIT:
							printf("Unit:\t\tBit\n");
							break;
						case SANE_UNIT_MM:
							printf("Unit:\t\tMillimeter\n");
							break;
						case SANE_UNIT_DPI:
							printf("Unit:\t\tDPI\n");
							break;
						case SANE_UNIT_PERCENT:
							printf("Unit:\t\tPercent\n");
							break;
						case SANE_UNIT_MICROSECOND:
							printf("Unit:\t\tMicrosecond\n");
							break;
					}

					switch (option->GetConstraintType()) {
						case SANE_CONSTRAINT_RANGE: {
							SANE_Range *range = option->GetConstraintRange();
							if (option->GetType() == SANE_TYPE_FIXED) {
								printf("Min:\t\t%f\n", SANE_UNFIX(range->min));
								printf("Max:\t\t%f\n", SANE_UNFIX(range->max));
								printf("Quant:\t\t%f\n", SANE_UNFIX(range->quant));
							} else {
								printf("Min:\t\t%d\n", range->min);
								printf("Max:\t\t%d\n", range->max);
								printf("Quant:\t\t%d\n", range->quant);
							}
							break;
						}
						case SANE_CONSTRAINT_WORD_LIST: {
							SANE_Word *word_list = option->GetConstraintWordList();
							SANE_Word word_list_length = *word_list;
							for (int index = 1; index <= word_list_length; index++) {
								if (option->GetType() == SANE_TYPE_FIXED) {
									printf("Word:\t\t%f\n", SANE_UNFIX(word_list[index]));
								} else {
									printf("Word:\t\t%d\n", word_list[index]);
								}
							}
							break;
						}
						case SANE_CONSTRAINT_STRING_LIST: {
							SANE_String_Const *string_list = option->GetConstraintStringList();
							for (int index = 0; string_list[index] != NULL; index++) {
								printf("String:\t\t%s\n", string_list[index]);
							}
							break;
						}
					}

					if (option->GetName()) {
						if (strcmp(option->GetName(), "mode") == 0) {
							SANE_String mode1 = option->SetValueString("Color");
							printf("Mode 1:\t\t%s\n", mode1);
							delete mode1;
							SANE_String mode2 = option->GetValueString();
							printf("Mode 2:\t\t%s\n", mode2);
							delete mode2;
						} else if (strcmp(option->GetName(), "compression") == 0) {
							SANE_String compression1 = option->SetValueString("None");
							printf("Compression 1:\t%s\n", compression1);
							delete compression1;
							SANE_String compression2 = option->GetValueString();
							printf("Compression 2:\t%s\n", compression2);
							delete compression2;
						} else if (strcmp(option->GetName(), "resolution") == 0) {
							printf("Resolution 1:\t%d\n", option->SetValueInt(300));
							printf("Resolution 2:\t%d\n", option->GetValueInt());
						}
					}
				}

				WINSANE_Params *params = device->GetParams();

				if (params) {
					printf("------------------------------------------------------------\n");
					printf("Format:\t\t%d\n", params->GetFormat());
					printf("Last Frame:\t%d\n", params->IsLastFrame());
					printf("Bytes per Line:\t%d\n", params->GetBytesPerLine());
					printf("Pixel per Line:\t%d\n", params->GetPixelsPerLine());
					printf("Lines:\t\t%d\n", params->GetLines());
					printf("Depth:\t\t%d\n", params->GetDepth());

					delete params;
				}

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

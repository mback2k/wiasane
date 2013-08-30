#include "stdafx.h"
#include "wiasane_opt.h"

HRESULT GetOptionValue(WIASANE_Context *context, SANE_String name, double *value)
{
	WINSANE_Option *option;

	option = context->device->GetOption(name);
	if (option) {
		switch (option->GetType()) {
			case SANE_TYPE_INT:
				*value = option->GetValueInt();
				return S_OK;

			case SANE_TYPE_FIXED:
				*value = SANE_UNFIX(option->GetValueFixed());
				return S_OK;

			case SANE_TYPE_STRING:
				return E_INVALIDARG;
		}
	}

	return E_NOTIMPL;
}

HRESULT GetOptionValue(WIASANE_Context *context, SANE_String name, char **value)
{
	WINSANE_Option *option;

	option = context->device->GetOption(name);
	if (option) {
		switch (option->GetType()) {
			case SANE_TYPE_STRING:
				*value = (char*) option->GetValueString();
				return S_OK;

			case SANE_TYPE_INT:
				return E_INVALIDARG;
		}
	}

	return E_NOTIMPL;
}


HRESULT SetOptionValue(WIASANE_Context *context, SANE_String name, double value)
{
	WINSANE_Option *option;
	SANE_Fixed should_fixed, actual_fixed;
	SANE_Int should_int, actual_int;

	option = context->device->GetOption(name);
	if (option) {
		switch (option->GetType()) {
			case SANE_TYPE_INT:
				should_int = (SANE_Int) value;
				actual_int = option->SetValueInt(should_int);
				return actual_int == should_int ? S_OK : E_FAIL;

			case SANE_TYPE_FIXED:
				should_fixed = (SANE_Fixed) SANE_FIX(value);
				actual_fixed = option->SetValueFixed(should_fixed);
				return actual_fixed == should_fixed ? S_OK : E_FAIL;

			case SANE_TYPE_STRING:
				return E_INVALIDARG;
		}
	}
	
	return E_NOTIMPL;
}

HRESULT SetOptionValue(WIASANE_Context *context, SANE_String name, char *value)
{
	WINSANE_Option *option;
	SANE_String should_str, actual_str;
	HRESULT hr;

	option = context->device->GetOption(name);
	if (option) {
		switch (option->GetType()) {
			case SANE_TYPE_STRING:
				should_str = (SANE_String) value;
				if (should_str) {
					actual_str = option->SetValueString(should_str);
					if (actual_str) {
						hr = strcmp(actual_str, should_str) == 0 ? S_OK : E_FAIL;
						delete actual_str;
						return hr;
					}
					return E_FAIL;
				}

			case SANE_TYPE_INT:
			case SANE_TYPE_FIXED:
				return E_INVALIDARG;
		}
	}
	
	return E_NOTIMPL;
}


HRESULT GetOptionValueInch(WIASANE_Context *context, SANE_String name, double *value)
{
	WINSANE_Option *option;
	SANE_Word *word_list, word;
	SANE_Range *range;

	option = context->device->GetOption(name);
	if (option) {
		switch (option->GetConstraintType()) {
			case SANE_CONSTRAINT_RANGE:
				range = option->GetConstraintRange();
				word = range->max;
				break;

			case SANE_CONSTRAINT_WORD_LIST:
				word_list = option->GetConstraintWordList();
				word = word_list[*word_list];
				break;

			default:
				return E_FAIL;
		}
		switch (option->GetType()) {
			case SANE_TYPE_INT:
				*value = word;
				break;

			case SANE_TYPE_FIXED:
				*value = SANE_UNFIX(word);
				break;

			default:
				return E_INVALIDARG;
		}
		switch (option->GetUnit()) {
			case SANE_UNIT_PIXEL:
				*value /= 96;
				return S_OK;

			case SANE_UNIT_MM:
				*value /= 25.4;
				return S_OK;
		}
	}

	return E_NOTIMPL;
}

HRESULT IsValidOptionValueInch(WIASANE_Context *context, SANE_String name, double value)
{
	WINSANE_Option *option;
	SANE_Word word;

	option = context->device->GetOption(name);
	if (option) {
		switch (option->GetUnit()) {
			case SANE_UNIT_PIXEL:
				value *= 96;
				break;

			case SANE_UNIT_MM:
				value *= 25.4;
				break;
		}
		switch (option->GetType()) {
			case SANE_TYPE_INT:
				word = (SANE_Int) value;
				break;

			case SANE_TYPE_FIXED:
				word = (SANE_Fixed) SANE_FIX(value);
				break;

			default:
				return E_INVALIDARG;
		}
		return option->IsValidValue(word) ? S_OK : E_INVALIDARG;
	}

	return E_NOTIMPL;
}

HRESULT SetOptionValueInch(WIASANE_Context *context, SANE_String name, double value)
{
	WINSANE_Option *option;

	option = context->device->GetOption(name);
	if (option) {
		switch (option->GetUnit()) {
			case SANE_UNIT_PIXEL:
				value *= 96;
				break;

			case SANE_UNIT_MM:
				value *= 25.4;
				break;
		}
		return SetOptionValue(context, name, value);
	}

	return E_NOTIMPL;
}

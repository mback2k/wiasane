#include "stdafx.h"
#include "wiasane_opt.h"

HRESULT GetOptionValue(WINSANE_Option *option, double *value)
{
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

	return E_NOTIMPL;
}

HRESULT GetOptionMaxValue(WINSANE_Option *option, double *value)
{
	SANE_Word *word_list, word;
	SANE_Range *range;
	int idx;

	switch (option->GetConstraintType()) {
		case SANE_CONSTRAINT_RANGE:
			range = option->GetConstraintRange();
			word = range->max;
			break;

		case SANE_CONSTRAINT_WORD_LIST:
			word_list = option->GetConstraintWordList();
			word = word_list[*word_list];
			for (idx = 1; idx <= *word_list; idx++) {
				word = max(word, word_list[idx]);
			}
			break;

		default:
			return E_FAIL;
	}
	switch (option->GetType()) {
		case SANE_TYPE_INT:
			*value = word;
			return S_OK;

		case SANE_TYPE_FIXED:
			*value = SANE_UNFIX(word);
			return S_OK;

		case SANE_TYPE_STRING:
			return E_INVALIDARG;
	}

	return E_NOTIMPL;
}

HRESULT GetOptionValue(WINSANE_Option *option, char **value)
{
	switch (option->GetType()) {
		case SANE_TYPE_STRING:
			*value = (char*) option->GetValueString();
			return S_OK;

		case SANE_TYPE_INT:
			return E_INVALIDARG;
	}

	return E_NOTIMPL;
}


HRESULT SetOptionValue(WINSANE_Option *option, double value)
{
	SANE_Fixed should_fixed, actual_fixed;
	SANE_Int should_int, actual_int;

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

	return E_NOTIMPL;
}

HRESULT SetOptionValue(WINSANE_Option *option, char *value)
{
	SANE_String should_str, actual_str;
	HRESULT hr;

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
	
	return E_NOTIMPL;
}


HRESULT GetOptionValueInch(WINSANE_Option *option, double *value)
{
	HRESULT hr;

	hr = GetOptionValue(option, value);
	if (hr != S_OK)
		return hr;

	switch (option->GetUnit()) {
		case SANE_UNIT_PIXEL:
			*value /= 96.0;
			return S_OK;

		case SANE_UNIT_MM:
			*value /= 25.4;
			return S_OK;
	}

	return E_NOTIMPL;
}

HRESULT GetOptionMaxValueInch(WINSANE_Option *option, double *value)
{
	HRESULT hr;

	hr = GetOptionMaxValue(option, value);
	if (hr != S_OK)
		return hr;

	switch (option->GetUnit()) {
		case SANE_UNIT_PIXEL:
			*value /= 96.0;
			return S_OK;

		case SANE_UNIT_MM:
			*value /= 25.4;
			return S_OK;
	}

	return E_NOTIMPL;
}

HRESULT IsValidOptionValueInch(WINSANE_Option *option, double value)
{
	SANE_Word word;

	switch (option->GetUnit()) {
		case SANE_UNIT_PIXEL:
			value *= 96.0;
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

HRESULT SetOptionValueInch(WINSANE_Option *option, double value)
{
	switch (option->GetUnit()) {
		case SANE_UNIT_PIXEL:
			value *= 96.0;
			break;

		case SANE_UNIT_MM:
			value *= 25.4;
			break;
	}

	return SetOptionValue(option, value);
}

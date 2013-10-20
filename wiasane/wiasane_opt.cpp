#include "wiasane_opt.h"

HRESULT GetOptionMaxValue(_In_ PWINSANE_Option option, _Out_ double *value)
{
	SANE_Word *word_list, word;
	SANE_Range *range;
	int idx;

	if (!option || !value)
		return E_INVALIDARG;

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

HRESULT GetOptionValueInch(_In_ PWINSANE_Option option, _Out_ double *value)
{
	HRESULT hr;

	if (!option || !value)
		return E_INVALIDARG;

	hr = option->GetValue(value);
	if (FAILED(hr))
		return hr;

	switch (option->GetUnit()) {
		case SANE_UNIT_PIXEL:
			*value /= 96.0;
			break;

		case SANE_UNIT_MM:
			*value /= 25.4;
			break;

		default:
			hr = E_NOTIMPL;
			break;
	}

	return hr;
}

HRESULT GetOptionMaxValueInch(_In_ PWINSANE_Option option, _Out_ double *value)
{
	HRESULT hr;

	if (!option || !value)
		return E_INVALIDARG;

	hr = GetOptionMaxValue(option, value);
	if (FAILED(hr))
		return hr;

	switch (option->GetUnit()) {
		case SANE_UNIT_PIXEL:
			*value /= 96.0;
			break;

		case SANE_UNIT_MM:
			*value /= 25.4;
			break;

		default:
			hr = E_NOTIMPL;
			break;
	}

	return hr;
}

HRESULT IsValidOptionValueInch(_In_ PWINSANE_Option option, _In_ double value)
{
	if (!option)
		return E_INVALIDARG;

	switch (option->GetUnit()) {
		case SANE_UNIT_PIXEL:
			value *= 96.0;
			break;

		case SANE_UNIT_MM:
			value *= 25.4;
			break;
	}

	return option->IsValidValue(value) ? S_OK : E_INVALIDARG;
}

HRESULT SetOptionValueInch(_In_ PWINSANE_Option option, _In_ double value)
{
	switch (option->GetUnit()) {
		case SANE_UNIT_PIXEL:
			value *= 96.0;
			break;

		case SANE_UNIT_MM:
			value *= 25.4;
			break;
	}

	return option->SetValue(value);
}

#ifndef LIBADC_H__
#define LIBADC_H__

// Convertion factor: 352/15 * Vref(mV)
#define VREF_FACTOR 71104

extern void ADC_PowerOn(void); 

extern void ADC_Convert(void); 

extern unsigned int ADC_Cleanup(void); 

#endif

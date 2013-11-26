#ifndef UWBIRDECIDERRESULT_H
#define UWBIRDECIDERRESULT_H

#include <vector>

#include "MiXiMDefs.h"
#include "Decider.h"

/**
 * @brief This class stores results from an UWBIR Decider.
 * It allows to pass to the MAC layer the demodulate bit values,
 * so that it (the MAC layer) can compare these demodulated bit values
 * with the bit values actually encoded by the MAC layer at the origin.
 *
 * @ingroup ieee802154a
 * @ingroup decider
 */
class MIXIM_API DeciderResultUWBIR : public DeciderResult {
public:
	DeciderResultUWBIR(bool isCorrect, std::vector<bool>* _decodedBits, double snr): DeciderResult(isCorrect), decodedBits(_decodedBits), snr(snr) { }

	~DeciderResultUWBIR() {
		decodedBits->clear();
		delete decodedBits;
	}

    // CSEM Jerome Rousselot
    const std::vector<bool>* getDecodedBits() const
        { return decodedBits; }


    double getSNR() const { return snr; }

private:
	std::vector<bool>* decodedBits;
	double snr;
	/** @brief Copy constructor is not allowed.
	 */
	DeciderResultUWBIR(const DeciderResultUWBIR&);
	/** @brief Assignment operator is not allowed.
	 */
	DeciderResultUWBIR& operator=(const DeciderResultUWBIR&);

};
#endif


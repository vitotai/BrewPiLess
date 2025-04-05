#if 1 // DerivePolynomialByController
#include "ExternalData.h"
#include "PolynomialRegression.h"



bool FormulaKeeper::getFormula(float coeff[4]){

    if(_numberOfPoints<2) return false;

    std::vector<float> x;
    std::vector<float> y;
    
    for (int xx = 0; xx < _numberOfPoints; xx++) {
        if((_ignoredMask & (1 << xx)) ==0){
            x.push_back(_calTilts[xx]);
            y.push_back(_calGravities[xx]);
            DBG_PRINTF("Regresson: %.3f, %4f\n",_calTilts[xx],_calGravities[xx]);
        }
    }
    int validcount = x.size();
    if(validcount < 2) return false;

    int order;
    if(validcount > 3 ) order=3;
    else if (validcount > 2 ) order=2;
    else order =1;

    // Do the regression.
    PolynomialRegression<float> polyreg;
    std::vector<float> coeffs;
    polyreg.fitIt( x, y, order, coeffs );

    // Print the coefficients.
    for ( int j = 0; j <= order; ++j ) {
        coeff[j] = coeffs[j];
    }
    for ( int j = order +1; j < 4; ++j ) {
        coeff[j] = 0;
    }
    return true;
}

bool FormulaKeeper::setTilt(float tilt,uint32_t time){
    _lastTiltTime = time;
    _lastTilt = tilt;
    
    DBG_PRINTF(" setTilt:%.4f\n",tilt);
    if(_lastGravity != INVALID_SG && _numberOfPoints < MaxNumberCalibrationPoints){
        addPoint(_lastTilt,_lastGravity);
        _lastGravity = INVALID_SG;
        return true;
    }
    return false;
}

bool FormulaKeeper::addGravity(float sg){

	DBG_PRINTF(" addGravity:%.4f\n",sg);

    if(_lastTilt != INVALID_TILT && _numberOfPoints < MaxNumberCalibrationPoints){
        addPoint(_lastTilt,sg);
        return true;
    }else{
        // wait unitl tilt value available
        _lastGravity = sg;
        return false;
    }
}

void FormulaKeeper::addPoint(float tilt,float sg){
        if(_numberOfPoints >= MaxNumberCalibrationPoints) return;
        int pos=0;

        while(pos<_numberOfPoints && _calTilts[pos]< tilt) pos++;

        if(pos < _numberOfPoints){
            for(int i=_numberOfPoints; i> pos;i--){
                _calTilts[i]= _calTilts[i-1];
                _calGravities[i] =_calGravities[i-1];
            }
        }
        _calTilts[pos]= tilt;
        _calGravities[pos] = sg;
        _numberOfPoints ++;        
        DBG_PRINTF("*** addPoint:%.4f,%f.4f @%d\n",tilt,sg,pos);
}

#endif
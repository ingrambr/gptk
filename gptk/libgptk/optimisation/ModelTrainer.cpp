/***************************************************************************
 *   AstonGeostats, algorithms for low-rank geostatistical models          *
 *                                                                         *
 *   Copyright (C) Ben Ingram, 2008-2009                                   *
 *                                                                         *
 *   Ben Ingram, IngramBR@Aston.ac.uk                                      *
 *   Neural Computing Research Group,                                      *
 *   Aston University,                                                     *
 *   Aston Street, Aston Triangle,                                         *
 *   Birmingham. B4 7ET.                                                   *
 *   United Kingdom                                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
 //
 // ModelTrainer abstract class.  Includes implementation of line minimiser
 // and minimum bracket algorithms which are based heavily on the Netlab
 // toolbox originally written in Matlab by Prof Ian Nabney
 //
 // History:
 //
 // 12 Dec 2008 - BRI - First implementation. 
 //
 
 
 
 
#include "ModelTrainer.h"

ModelTrainer::ModelTrainer(Optimisable& m) : model(m)
{
	// default training options
	display = true;
	errorTolerance = 1.0e-6;
	parameterTolerance = 1.0e-4;
	
	gradientCheck = true; // perform a gradient check
	analyticGradients = true; // use analytic gradient calculations
	
	functionEvaluations = 0; // reset function counter
	gradientEvaluations = 0; // reset gradient counter
	functionValue = 0.0; // current function value
	
	lineMinimiserIterations = 10;
	lineMinimiserParameterTolerance = 1.0e-4;
	
	maskSet = false; // do we want to mask certain parameters
	
	epsilon = 1.0e-6;	// delta for finite difference approximation
}

ModelTrainer::~ModelTrainer()
{
}


double ModelTrainer::errorFunction(vec params)
{
	functionEvaluations++;
	setParameters(params);
	return(model.objective());
}

vec ModelTrainer::errorGradients(vec params)
{
	if(analyticGradients)
	{
		gradientEvaluations++;
		setParameters(params);
		return(model.gradient());
	}
	else
	{
		return(numericalGradients(params));
	}
}

vec ModelTrainer::numericalGradients(const vec params)
{
	setParameters(params);
	vec g;
	int numParams = params.size();

	g.set_size(numParams);
	for(int i=0; i < numParams; i++)
	{
		g(i) = calculateNumericalGradient(i, params);
	}
	return(g);
}

double ModelTrainer::calculateNumericalGradient(const int parameterNumber, const vec params)
{
	vec xNew;
	double fplus, fminus;
	
	xNew = params;
	xNew(parameterNumber) = xNew(parameterNumber) + epsilon;
	cout << "Param + eps: " << xNew(parameterNumber) + epsilon << endl;
		
	fplus = errorFunction(xNew);
	cout << "Err: " << fplus << endl;
		
	xNew = params;
	xNew(parameterNumber) = xNew(parameterNumber) - epsilon;
	cout << "Param - eps: " << xNew(parameterNumber) + epsilon << endl;
	fminus = errorFunction(xNew);
	cout << "Err: " << fminus << endl << endl;
	
	return (0.5 * ((fplus - fminus) / epsilon));
}
	
void ModelTrainer::setParameters(const vec p)
{
	if(maskSet)
	{
		int pos = 0;
		vec maskedParameters = model.getParametersVector();
		for(int i = 0 ; i < optimisationMask.size() ; i++)
		{
			if(optimisationMask(i) == true)
			{
				maskedParameters(i) = p(pos);
				pos++;
			}
		}
		model.setParametersVector(maskedParameters);
	}
	else
	{
		model.setParametersVector(p);
	}
}

vec ModelTrainer::getParameters()
{
	if(maskSet)
	{
		vec p = model.getParametersVector();
		vec maskedParameters;
		assert(optimisationMask.size() == p.size());
		for(int i = 0 ; i < optimisationMask.size() ; i++)
		{
			if(optimisationMask(i) == true)
			{
				maskedParameters = concat(maskedParameters, p(i));
			}
		}
		return maskedParameters;
	}
	else
	{
		return model.getParametersVector();
	}
}

void ModelTrainer::checkGradient()
{
	vec xOld = getParameters();
	vec gNew = model.gradient();
	int numParams = gNew.size();
	int pos = 0;
	
	double delta;

	cout << "==========================" << endl;
	cout << "GRADCHECK" << endl;
	cout << "     Delta, Analytic, Diff" << endl;
	cout << "--------------------------" << endl;
	

	for(int i=0; i < numParams; i++)
	{
		cout << "#" << i << " ";
		if(maskSet)
		{
			if(optimisationMask(i) == true)
			{
				delta = calculateNumericalGradient(pos, xOld);
				pos = pos + 1;
				cout << " ";
			}
			else
			{
				delta = 0.0;
				gNew(i) = 0.0;
				cout << "x";
			}
		}
		else
		{
			delta = calculateNumericalGradient(i, xOld);
		}
		cout << " " << delta << ", " << gNew(i) << ", " << abs(delta - gNew(i)) <<endl;

	}
	cout << "==========================" << endl;

}

void ModelTrainer::Summary() const
{
	cout << "================================================" << endl;
	cout << "Training summary     : " << algorithmName << endl;
	cout << "------------------------------------------------" << endl;
	cout << "Error tolerance      : " << errorTolerance << endl;
	cout << "Parameter tolerance  : " << parameterTolerance << endl;
	cout << "Function evaluations : " << functionEvaluations << endl;
	cout << "Gradient evaluations : " << gradientEvaluations << endl;
	cout << "Function value       : " << functionValue << endl;
	cout << "================================================" << endl;
}

double ModelTrainer::lineFunction(vec param, double lambda, vec direction)
{
	double f;
	vec xOld = getParameters();
	f = errorFunction(param + (lambda * direction));
	setParameters(xOld);
	return(f);
}

void ModelTrainer::lineMinimiser(double &fx, double &x, double fa, vec params, vec direction)
{
	double br_min, br_mid, br_max;
	double w, v, e, d = 0, r, q, p, u, fw, fu, fv, xm, tol1;

	bracketMinimum(br_min, br_mid, br_max, 0.0, 1.0, fa, params, direction);

	w = br_mid;
	v = br_mid;
	x = v;
	e = 0.0;
	fx = lineFunction(params, x, direction);
	fv = fx;
	fw = fx;

	for(int n = 1; n <= lineMinimiserIterations; n++)
	{
		xm = 0.5 * (br_min + br_max);
		tol1 = TOL * abs(x) + TINY;
 
		if((abs(x - xm) <= lineMinimiserParameterTolerance) & ((br_max - br_min) < (4 * lineMinimiserParameterTolerance)))
		{
			functionValue = fx;
			return;
		}
		
		if (abs(e) > tol1)
		{
			r = (fx - fv) * (x - w);
			q = (fx - fw) * (x - v);
			p = ((x - v) * q) - ((x - w) * r);
			q = 2.0 * (q - r);

			if (q > 0.0)
			{
				p = -p;
			}
			
			q = abs(q);

			if ((abs(p) >= abs(0.5 * q * e)) | (p <= (q * (br_min - x))) | (p >= (q * (br_max - x))))
			{
				if (x >= xm)
				{
					e = br_min - x;
				}
				else
				{
					e = br_max - x;
				}
		      d = CPHI * e;
		   }
			else
			{
				e = d;
				d = p / q;
				u = x + d;
				if (((u - br_min) < (2 * tol1)) | ((br_max - u) < (2 * tol1)))
				{
					d = sign(xm - x) * tol1;
				}
			}
		}
		else
		{
			if (x >= xm)
			{
				e = br_min - x;
			}
			else
			{
				e = br_max - x;
			}
			d = CPHI * e;
		}
	
		if (abs(d) >= tol1)
		{
			u = x + d;
		}
		else
		{
			u = x + sign(d) * tol1;
		}
		
		fu = lineFunction(params, u, direction);

		if (fu <= fx)
		{
			if (u >= x)
			{
				br_min = x;
			}
			else
			{
				br_max = x;
			}
			v = w; w = x;x = u;
			fv = fw; fw = fx;	fx = fu;
		}
		else
		{
			if (u < x)
			{
				br_min = u;
			}   
			else
			{
				br_max = u;
			}
		
			if ((fu <= fw) | (w == x))
			{
				v = w; w = u;
				fv = fw; fw = fu;
			}
			else
			{
				if ((fu <= fv) | (v == x) | (v == w))
				{
					v = u; fv = fu;
				}
			}
// add an option for lineMinimiser display?
//			if (display)
//			{
//				//fprintf(1, 'Cycle %4d  Error %11.6f\n', n, fx);
//				cout << "Line Minimiser: Cycle " << n;
//				cout << "  Error " << fx << endl;
//			}
		}
	}
}

void ModelTrainer::bracketMinimum(double &br_min, double &br_mid, double &br_max, double a, double b, double fa, vec params, vec direction)
{
	double c, fc, r, q, u, ulimit, fu = 0.0;
	double fb = lineFunction(params, b, direction);

	bool bracket_found;

	const double max_step = 10.0;

	if(fb > fa)
	{
		c = b;
		b = a + (c - a) / PHI;
		fb = lineFunction(params, b, direction);
		
		while (fb > fa)
		{
			c = b;
    		b = a + (c - a) / PHI;
    		fb = lineFunction(params, b, direction);
		}
	}
	else
	{
		c = b + PHI * (b - a);
		fc = lineFunction(params, c, direction);

		bracket_found = false;
  
		while (fb > fc)
		{
			r = (b - a) * (fb - fc);
			q = (b - c) * (fb - fa);
			u = b - ((b - c) * q - (b - a) * r) / (2.0 * (sign(q - r) * max(abs(q - r), TINY)));

			ulimit = b + max_step * (c - b);
    
			if ((b - u) * (u - c) > 0.0)
			{
				fu = lineFunction(params, u, direction);
				if (fu < fc)
				{
					br_min = b;	br_mid = u;	br_max = c;
					return;
				}
 				else
 				{
 					if (fu > fb)
 					{
						br_min = a;	br_mid = c;	br_max = u;
						return;
					}
				}
				u = c + PHI * (c - b);
			}
			else
			{
				if ((c - u) * (u - ulimit) > 0.0)
				{
					fu = lineFunction(params, u, direction);

					if (fu < fc)
					{
						b = c;
						c = u;
						u = c + PHI * (c - b);
					}
					else
					{
						bracket_found = true;
					}
				}
				else
				{
					if ((u - ulimit) * (ulimit - c) >= 0.0)
					{
						u = ulimit;
					}
					else
					{
						u = c + PHI * (c - b);
					}
				}
			}
			
			if(!bracket_found)
			{
				fu = lineFunction(params, u, direction);
			}
			a = b; b = c; c = u;
			fa = fb; fb = fc; fc = fu;
		}
	}
	
	br_mid = b;

	if(a < c)
	{
		br_min = a; br_max = c;
	}
	else
	{
		br_min = c; br_max = a;
	}
}





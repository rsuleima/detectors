void setStyle()
{
	gStyle->SetPadLeftMargin(-0.08);
	gStyle->SetPadRightMargin(-0.04);
	gStyle->SetPadTopMargin(-0.06);
	gStyle->SetPadBottomMargin(-0.06);
	
	gStyle->SetPalette(1);
	gStyle->SetOptStat(0);
	gStyle->SetOptTitle(0);
	gStyle->SetFrameFillColor(kWhite);
	gStyle->SetCanvasColor(kWhite);
	gStyle->SetPadColor(kWhite);

	gStyle->SetCanvasBorderMode(0);
	gStyle->SetFrameBorderMode(0);
	gStyle->SetPadBorderMode(0);
}





// formula to calculate the window percentage increase
double windowy(double *x, double *par)
{
	// x[0] is the position along the side wall
	// par[0] is cut / not cut (0 or 1)	
	// par[1] is nose / no nose (0 or 1)
	double nose = par[1] * NOSEY;
	
	double sideLength = 1; // side is 1 m long
	
	if(par[0] == 0)
	{
		double m = nose/WD;
		if(x[0] > 0 && x[0] < WD)
			return -(WY0 - sqrt(WR*WR - pow(x[0] - WX0, 2))) + nose -m*x[0] ;
		else
			return 0;
	}
	else
	{
		double m = nose/WD2;
		if(x[0] > 0 && x[0] < WD2)
			return (-(WY02 - sqrt(WR*WR - pow(x[0] - WX02, 2))) + nose -m*x[0] ) / sideLength;
		else
			return 0;

	}
}


// pure photon yield as from Tamm's formula
double dndxdl(double *x, double *par)
{
	// x[0] is the wavelength
	// x[1] is momentum
	// par[0] is the particle mass
	// par[1] is the mirror reflectivity: 0 is 100%, 1 is LTCC, 2 is ECI witness 3 is for ECI sample
	// par[2] is the PMT quantum efficiency: 0 for 100%, 1 for std, 2 for UV, 3 for quartz
	// par[3] is the gas transparency: 0 for 100%, 1 c4f10

	double l = x[0];
  double p = x[1];
  double m = par[0];
	
	// refraction index as a function of lambda
	double ri = 1 + pow(10, -6)*p1_S/(pow(p2_S, -2) - pow(l, -2));
	
	// uncomment below for quartz radiator
	// double l2 = l/1000.0;
	// double ri = sqrt(1.28604141 + (1.07044083*l2*l2)/(l2*l2-0.0100585997) + (1.10202242*l2*l2)/(l2*l2-100.0));

	double c = 2*PI*alpha / (l/10000000.0*l);  // one of the lambda in cm. The other doesn't matter cause I integrate over it 
	double beta = sqrt(p*p/(m*m + p*p));
	
	
	double refl   = 0;
	double qe     = 0;
	double transp = 0;
	
	
	// mirror reflectivity choices
	if(par[1] == 0) refl = 1;
	if(par[1] == 1) refl = interpolate(l, "ltcc_refl");
	if(par[1] == 2) refl = interpolate(l, "ecis_witn");
	if(par[1] == 3) refl = interpolate(l, "ecis_samp");
	

	// quantum efficiency choices
	if(par[2] == 0) qe = 1;
	if(par[2] == 1) qe = interpolate(l, "stdPMT");
	if(par[2] == 2) qe = interpolate(l, "uvgPMT");
	if(par[2] == 3) qe = interpolate(l, "qtzPMT");
		
	// transparency
	if(par[3] == 0) transp = 1;
	if(par[3] == 1) transp = interpolate(l, "c4f10t");;
	
	
	// must be above threshold
	// reflectivity is counted twice cause two reflections
	if(beta*beta*ri*ri>1)		
		return refl*refl*refl*qe*transp*c*(1 - 1/(beta*beta*ri*ri));
	else
		return 0;

}

// interpolate linearly table values
double interpolate(double x, string what)
{
	// out of range
	if(x < 190 || x > 650)
		return 0;
	
	
	// getting wavelength indexes
	
	int i1 = floor((x - 190.0)/10.0);
	int i2 = i1 + 1;
	
	double *data;

	if(what == "n")
		data = c4f10n;
	else if(what == "stdPMT")
		data = stdPmt_qe;
	else if(what == "uvgPMT")
		data = uvgPmt_qe;
	else if(what == "qtzPMT")
		data = qtzPmt_qe;
	else if(what == "c4f10t")
		data = c4f10t;
	else if(what == "ltcc_refl")
		data = ltcc_refl;
	else if(what == "eciw_witn")
		data = eciw_witn;
	else if(what == "ecis_samp")
		data = ecis_samp;
	else
		cout << " No data selected in interpolation routine. This will crash the macro" << endl;

	if(i2 >= NP)
		return data[NP-1];

	double dx = lambda[i2] - lambda[i1];
	double dy = data[i2] - data[i1];
	
	double m = dy/dx;
	double b = data[i1] - m*lambda[i1];
	
	return m*x + b;	
}





void change_particle()
{
	PART++;
	if(PART==3) PART = 0;
	
	cout << " Particle selected: " << pname[PART] << endl;
}

void change_mirror()
{
	MIRROR++;
	if(MIRROR==4) MIRROR = 0;
	
	cout << " mirror selected: " << mirname[MIRROR] << endl;
}

void change_gas()
{
	GAS++;
	if(GAS==2) GAS = 0;
	
	cout << " Gas selected: " << gasname[GAS] << endl;
}

void change_pmt()
{
	PMT++;
	if(PMT==4) PMT = 0;
	
	cout << " PMT selected: " << pmtname[PMT] << endl;
}




void integrate_yield()
{

	double *dummy = 0;
	double *mom;
  
	TF2 *momF;
	
	if(PART == 0)
	{
		mom = electron_m;
		momF = dndxdlFelectron;
    momF->SetParameter(0, eMass);
	}
	if(PART == 1)
	{
		mom = pion_m;
		momF = dndxdlFpion;
    momF->SetParameter(0, piMass);
	}
	if(PART == 2)
	{
		mom = kaon_m;
		momF = dndxdlFkaon;
    momF->SetParameter(0, kMass);
	}

	momF->SetParameter(1, MIRROR);
	momF->SetParameter(2, PMT);
	momF->SetParameter(3, GAS);

	double ngammas[MNP];
	for(int i=0; i<MNP; i++)
	{
		TF12 *temp = new TF12("temp", momF, mom[i], "x");
		ngammas[i] = temp->Integral(190, 650, dummy, 0.1);
		cout << pname[PART] << " momentum: " << mom[i] << "   n gammas: " << ngammas[i]
         << " mirror: " << mirname[MIRROR] <<  "  pmt: " << pmtname[PMT] <<  endl;
		
		if(PART == 0)
			electron_n[i] = ngammas[i];
		if(PART == 1)
			pion_n[i] = ngammas[i];
		if(PART == 2)
			kaon_n[i] = ngammas[i];

		delete temp;
	}
	
	delete dummy;
}





void print_all_yields()
{
	PRINT = ".gif";

	for(int p=0; p<3; p++)
	{
		PART = p;
		
		for(int m=0; m<4; m++)
		{
			MIRROR = m;
		
		}
		plot_yields();
		draw_W();
		
	}

}








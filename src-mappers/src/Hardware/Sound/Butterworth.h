class Butterworth {
private:
	double *A, *d1, *d2, *w0, *w1, *w2;
	int n;
	void flush();
public:
	double process(double);
	void recalc(int, double, double);
	Butterworth(int, double, double);
	~Butterworth();
};

class LPF_RC {
protected:    
	double	a0;
	double	b1;
	double	z1;
public:
	LPF_RC();
	LPF_RC(double);
	void	setFc (double);
	double	process (double);  
};

class HPF_RC: public LPF_RC {
protected:    
public:
	HPF_RC();
	HPF_RC(double);
	double	process (double);  
};


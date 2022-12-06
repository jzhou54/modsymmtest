#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
NumericVector Cquantile(NumericVector x, NumericVector q) {
  NumericVector y = clone(x);
  std::sort(y.begin(), y.end());
  return y[x.size()*(q - 0.000000001)];
}


//' @export
// [[Rcpp::export]]
double get_V(NumericVector x_,
             Nullable<NumericVector> y_ = R_NilValue) {
  
  NumericVector x = clone(x_);
  int nx = x.size();
  
  if (y_.isNotNull()) {
    NumericVector y(y_);
    int ny = y.size();
    bool same = (nx == ny);
    if(same) {
      for (int i=0; i<nx; i++){
        x[i] = x_[i] - y[i];
      };
    }else stop("'x' and 'y' must have the same length");
  };
  
  // basic summary statistic;
  double m = mean(x);
  double sigma2 = var(x);
  double sd = sqrt(sigma2);
  NumericVector xc = x - m;
  
  // Function f("wilcox.test");
  // List wilres = f(x, Named("mu", m));
  // double STATISTIC = wilres[0];
  
  // Tn selection
  NumericVector quan = {0.25, 0.75};
  NumericVector r = Cquantile(x, quan);
  double h = (r[1] - r[0]) / 1.34;
  double Tn = log(nx)/( 3 * 1.06 * std::min(sd, h));
  
  
  // Estimation of tau
  NumericVector xs = xc.sort();
  IntegerVector S1 = seq(1, nx);
  NumericVector holding = (as<NumericVector>(S1)*1);
  double hat_tau = sum(xs*holding)/pow(nx, 2);
  
  // Estimation of theta
  double theta_res_p1 = 0;
  double theta_res_p2 = 0;
  for (int i = 0; i < nx; i++) {
    for (int j = 0; j < nx; j++) {
      theta_res_p2 = theta_res_p2 +  sin(2*M_PI*(x[i]+x[j]-2*m)) / (M_PI*(x[i]+x[j]-2*m));
      if (i != j) {
        theta_res_p1 = theta_res_p1 + sin(2*M_PI*(x[i]-x[j])) / (M_PI*(x[i]-x[j]));
      };
    };
  };
  
  double hat_theta = theta_res_p1/pow(nx, 2) + 2*Tn/ nx + theta_res_p2/pow(nx,2);
  
  
  
  // Asymptotic mean and variance
  double V1 = nx*(nx+1)*(2*nx+1)/24;
  double V2 = nx*(nx-1)*(nx-3) * hat_theta * hat_tau;
  double V3_p1 = nx-4;
  double V3 = V3_p1*(nx-3)*(nx-2)*(nx-1)*sigma2/(4*nx)*pow(hat_theta,2);
  double V = V1 - V2 + V3;
  
  // output
  return V;
  
}

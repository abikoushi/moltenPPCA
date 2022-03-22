#include <RcppArmadillo.h>
#include <math.h>
using namespace Rcpp;
// [[Rcpp::depends(RcppArmadillo)]]
// [[Rcpp::plugins(cpp11)]]

static const double logsqrt2pi = log(2*M_PI)/2;

arma::vec r_norm_vec(arma::vec mean, arma::vec sd) {
  int N = mean.n_rows;
  arma::vec epsilon(N);
  epsilon.randn();
  return mean + sd%epsilon;
}

arma::mat mysum_t(int n, arma::uvec xi, arma::uvec xp, arma::mat lam) {
  arma::mat out = arma::zeros<arma::mat>(n,lam.n_cols);
  for(int i=0; i<xp.n_rows-1; i++){
    for(int j=xp[i];j<xp[i+1];j++){
      out.row(i) += lam.row(xi[j]); 
    }
  }
  return out;
}

arma::mat mysum(int n, arma::uvec xi, arma::uvec xp, arma::mat lam) {
  arma::mat out = arma::zeros<arma::mat>(n,lam.n_cols);
  for(int i=0; i<xp.n_rows-1; i++){
    for(int j=xp[i];j<xp[i+1];j++){
      out.row(xi[j]) += lam.row(i); 
    }
  }
  return out;
}

// [[Rcpp::export]]
arma::mat myprod(int n, arma::uvec xi, arma::uvec xp, arma::mat lam) {
  arma::mat out = arma::ones<arma::mat>(n,lam.n_cols);
  for(int i=0; i<xp.n_rows-1; i++){
    for(int j=xp[i];j<xp[i+1];j++){
      out.row(xi[j]) %= lam.row(i); 
    }
  }
  return out;
}

arma::mat myprod_skip(int n, arma::uvec xi, arma::uvec xp, arma::mat lam, int start, int end) {
  arma::mat out = arma::ones<arma::mat>(n,lam.n_cols);
  for(int i=0; i<xp.n_rows-1; i++){
    if(i<start|i>end){
      for(int j=xp[i];j<xp[i+1];j++){
        out.row(xi[j]) %= lam.row(i);
      } 
    }
  }
  return out;
}

void sample_mu(arma::mat & mu, arma::vec y, int N, arma::uvec xi, arma::uvec xp, arma::uvec varind, int K, int L, double lambda, double tau){
  for(int k=0; k<K; k++){
    for(int l=0; l<L; l++){
      arma::mat mu_l = mu;
      mu_l.shed_col(l);
      arma::vec mu2x = myprod_skip(N, xi, xp, pow(mu.col(l),2), varind[k], varind[k+1]);
      arma::vec den = mysum_t(varind[k+1]-varind[k], xi, xp.rows(varind[k],varind[k+1]), mu2x);
      arma::vec mux = myprod_skip(N, xi, xp, mu.col(l), varind[k], varind[k+1]);
      arma::vec resid = y - sum(myprod(N, xi, xp, mu_l),1);
      arma::vec muhat = (mysum_t(varind[k+1]-varind[k], xi, xp.rows(varind[k],varind[k+1]), resid%mux))/(den + tau/lambda);
      mu.col(l).rows(varind[k],varind[k+1]-1) = r_norm_vec(muhat, sqrt(1/(lambda*den + tau)));
    }
  }
}

void up_ms(arma::mat & m, arma::mat & s2, arma::vec y, int N, arma::uvec xi, arma::uvec xp, arma::uvec varind, int K, int L, double lambda, double tau){
  for(int k=0; k<K; k++){
    for(int l=0; l<L; l++){
      arma::mat mu_l = m;
      mu_l.shed_col(l);
      arma::vec mu2x = myprod_skip(N, xi, xp, pow(m.col(l),2) + s2.col(l), varind[k], varind[k+1]);
      arma::vec den = mysum_t(varind[k+1]-varind[k], xi, xp.rows(varind[k],varind[k+1]), mu2x);
      arma::vec mux = myprod_skip(N, xi, xp, m.col(l), varind[k], varind[k+1]);
      arma::vec resid = y - sum(myprod(N, xi, xp, mu_l),1);
      m.col(l).rows(varind[k],varind[k+1]-1) = (mysum_t(varind[k+1]-varind[k], xi, xp.rows(varind[k],varind[k+1]), resid%mux))/(den + tau/lambda);
      s2.col(l).rows(varind[k],varind[k+1]-1) = 1/(lambda*den + tau);
    }
  }
}

// [[Rcpp::export]]
List doGibbs(arma::vec y,
             arma::uvec xi,
             arma::uvec xp,
             arma::uvec varind,
             int D,
             int L,
             int iter=1000,
             double lambda=1,
             double tau=1){
  int N = y.n_rows;
  int K = varind.n_rows - 1;
  arma::mat mu = arma::randn<arma::mat>(D,L);
  arma::mat loglik = arma::zeros<arma::mat>(N,iter);
  arma::cube mu_s(D,L,iter);
  // musamp.fill(0);
  for(int i=0; i<iter; i++){
    sample_mu(mu, y, N, xi, xp, varind, K, L, lambda, tau);
    arma::vec tmp = pow(y - sum(myprod(N, xi, xp, mu),1),2)/2;
    loglik.col(i) = -sqrt(lambda)*tmp + std::log(lambda)/2 - logsqrt2pi;
    mu_s.slice(i) = mu;
    //     double bhat = sum(tmp)+b;
    //     lambda = randg(arma::distr_param(ahat, 1/bhat));
  }
  return List::create(Named("mu") = mu_s, Named("loglik") = loglik);
}

// [[Rcpp::export]]
List doVB(arma::vec y,
             arma::uvec xi,
             arma::uvec xp,
             arma::uvec varind,
             int D,
             int L,
             int iter=100,
             double lambda=1,
             double tau=1){
  int N = y.n_rows;
  int K = varind.n_rows - 1;
  arma::mat m = arma::randn<arma::mat>(D,L);
  arma::mat s2 = arma::ones<arma::mat>(D,L);
  arma::vec ll = arma::zeros<arma::vec>(iter);
  for(int i=0; i<iter; i++){
    up_ms(m, s2, y, N, xi, xp, varind, K, L, lambda, tau);
    arma::vec f = sum(myprod(N, xi, xp, m),1);
    arma::vec f2 = sum(myprod(N, xi, xp, pow(m,2) + s2),1);
    double lq = arma::as_scalar(accu(0.5*log(s2) + 0.5)/(D*L));
    // double lp = 0.5*log(tau) + 0.5 + logsqrt2pi;
    ll.row(i) = mean(0.5*log(lambda)+(0.5*pow(y,2) - y%f + 0.5*f2)*sqrt(lambda))-lq;
  }
  return List::create(Named("mean") = m, Named("sd") = sqrt(s2), Named("logloss") = ll);
}
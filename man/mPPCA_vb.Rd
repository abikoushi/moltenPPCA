\name{mPPCA_vb}
\alias{mPPCA_vb}
\alias{mPPCA_vb.default}
\alias{mPPCA_vb.formula}
\title{
Estimates the model of molten PPCA using variational Bayes
}
\description{
\code{mPPCA_vb} is used to fit molten PPCA.
}
\usage{
mPPCA_vb(...)


mPPCA_vb.formula(formula,
                 data = parent.frame(),
                 L,
                 iter=200,
                 lambda=1,
                 tau=1)
                 
                 
mPPCA_vb.default(y, 
                 X,
                 L,
                 iter=200,
                 lambda=1,
                 tau=1)
}
%- maybe also 'usage' for other objects documented here.
\arguments{
  \item{y}{
  response variable
}
  \item{X}{
  sparse design matrix
}
  \item{formula}{
  an object of class "formula": a symbolic description of the model to be fitted.
}
  \item{data}{	
  an optional data frame, containing the variables in the formula.
}
  \item{X}{
  number of componets
}
  \item{iter}{
  number of iteration
}
  \item{lambda}{
  precision parameter (inverse of variance)
}
  \item{tau}{
  precision parameter of prior distribution
}
  \item{\dots}{
  additional arguments to be passed to the low level fitting functions 
}
}
\value{
 \item{mean }{mean of the variational posterior (normal distribution)}
 \item{sd }{standard deviation of the variational posterior (normal distribution)}
 \item{logloss }{log-probabilty evalueted by current model}
 \item{indices }{indices}
}
\author{
Ko ABE
}

\seealso{
\code{\link{mNMF_vb}}
}
\examples{
### see vignette
}
% Add one or more standard keywords, see file 'KEYWORDS' in the
% R documentation directory (show via RShowDoc("KEYWORDS")):
% \keyword{ ~kwd1 }
% \keyword{ ~kwd2 }
% Use only one keyword per line.
% For non-standard keywords, use \concept instead of \keyword:
% \concept{ ~cpt1 }
% \concept{ ~cpt2 }
% Use only one concept per line.

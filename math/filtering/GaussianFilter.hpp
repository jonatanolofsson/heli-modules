#ifndef SYS_MATH_FILTERING_GAUSSIAN_FILTER_HPP_
#define SYS_MATH_FILTERING_GAUSSIAN_FILTER_HPP_

#include <sys/math/filtering/types.hpp>
#include <os/mem/ProtectedData.hpp>

namespace sys {
    namespace math {
        template<typename M, typename S = Scalar>
        struct GaussianFilter : public os::ProtectedClass {
            typedef S Scalar;
            typedef typename Covariance<Scalar, M::nofStates>::type CovarianceMatrix;
            typedef typename StateVector<Scalar, M::nofStates>::type States;
            typedef typename ControlVector<Scalar, M::nofControls>::type Controls;
            typedef States Reference;
            typedef M Model;
            typedef GaussianFilter<M,S> Self;
            CovarianceMatrix covariance;
            States state;
            Controls controls;

            explicit GaussianFilter() {
                state.setZero();
                covariance.setZero();
            }

            GaussianFilter(const Self& f)
                : covariance(f.covariance)
                , state(f.state)
                , controls(f.controls)
            {}
        };

        template<typename Sensor>
        struct Measurement : public Sensor {
            typedef Sensor Model;
            Matrix<typename Sensor::Scalar, Sensor::nofMeasurements, 1> z;
            Matrix<typename Sensor::Scalar, Sensor::nofMeasurements, Sensor::nofMeasurements> R;
        };
    }
}

#endif
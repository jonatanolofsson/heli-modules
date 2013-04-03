#pragma once
#ifndef SYS_MATH_MODELS_SENSORS_GPS_HPP_
#define SYS_MATH_MODELS_SENSORS_GPS_HPP_

#include <sys/math/constants.hpp>
#include <sys/math/filtering/GaussianFilter.hpp>
#include <Eigen/Core>
#include <os/com/Dispatcher.hpp>
#include <sys/types.hpp>

namespace sys {
    namespace math {
        namespace models {
            using namespace Eigen;
            template<typename ModelDescription>
            struct Gps {
                typedef Gps<ModelDescription> Self;
                typedef typename ModelDescription::Scalar Scalar;
                enum state {
                    x = 0,
                    y = 1,
                    z = 2,

                    vx = 3,
                    vy = 4,
                    vz = 5,

                    nofMeasurements = 6
                };

                enum states {
                    position = x,
                    velocity = vx
                };

                typedef Matrix<Scalar, nofMeasurements, 1> MeasurementVector;
                typedef Matrix<Scalar, nofMeasurements, nofMeasurements> CovarianceMatrix;
                static MeasurementVector measurement(const typename ModelDescription::States& state) {
                    typedef typename ModelDescription::StateDescription states;
                    MeasurementVector m;
                    m[x] = state[states::x];
                    m[y] = state[states::y];
                    m[z] = state[states::z];

                    m[vx] = state[states::vx];
                    m[vy] = state[states::vy];
                    m[vz] = state[states::vz];

                    return m;
                }

                static CovarianceMatrix cov;
                static const CovarianceMatrix& covariance(const typename ModelDescription::States& state) { return cov; }

                static Matrix<Scalar, nofMeasurements, ModelDescription::nofStates>
                jacobian(const typename ModelDescription::States&) {
                    typedef typename ModelDescription::StateDescription states;
                    typedef Matrix<Scalar, nofMeasurements, ModelDescription::nofStates> JacobianMatrix;
                    JacobianMatrix J;
                    J.setZero();

                    J(x, states::x) = 1;
                    J(y, states::y) = 1;
                    J(z, states::z) = 1;

                    J(vx, states::vx) = 1;
                    J(vy, states::vy) = 1;
                    J(vz, states::vz) = 1;

                    return J;
                }


                template<typename PostOffice>
                static void packager(const typename PostOffice::Message& msg) {
                    USING_XYZ
                    struct gpsOverlay {
                        float pos[3];
                        float vel[3];
                    };
                    if(msg.size == sizeof(gpsOverlay)) {
                        gpsOverlay* gps = (gpsOverlay*)&msg.msg;
                        math::GaussianMeasurement<Self> m;
                        m.z[x] = gps.pos[X];
                        m.z[y] = gps.pos[Y];
                        m.z[z] = gps.pos[Z];
                        m.z[vx] = gps.vel[X];
                        m.z[vy] = gps.vel[Y];
                        m.z[vz] = gps.vel[Z];
                        os::yield(m);
                    }
                }
            };

            template<typename ModelDescription> typename Gps<ModelDescription>::CovarianceMatrix
            Gps<ModelDescription>::cov = (Gps<ModelDescription>::MeasurementVector() <<
                1.0, 1.0, 1.0, 1.0, 1.0, 1.0
            ).finished().asDiagonal();
        }
    }
}

#endif

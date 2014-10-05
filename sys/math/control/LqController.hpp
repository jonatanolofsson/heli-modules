#pragma once
#ifndef SYS_MATH_LQ_CONTROLLER_HPP_
#define SYS_MATH_LQ_CONTROLLER_HPP_

#include <Eigen/Core>
#include <Eigen/QR>
#include <Eigen/LU>
#include <Eigen/Eigenvalues>
#include <sys/types.hpp>
#include <iostream>


#ifndef P_DELTA_EPSILON
    #define P_DELTA_EPSILON 1e-4
#endif

namespace sys {
    namespace math {
        using namespace Eigen;
        template<typename ModelDescription, int EXTRA_STATES = 0, int EXTRA_CONTROLS = 0>
        class LqController {
            public:
                typedef LqController<ModelDescription, EXTRA_STATES, EXTRA_CONTROLS> Self;
                typedef typename ModelDescription::Scalar Scalar;
                static const int nofStates = ModelDescription::nofStates + EXTRA_STATES;
                static const int nofControls = ModelDescription::nofControls + EXTRA_CONTROLS;
                typedef Matrix<Scalar, nofStates, nofStates> StateMatrix; ///< A state (propagation) matrix describes how the model state evolves in time, given no control
                typedef Matrix<Scalar, nofStates, nofControls> ControlMatrix; ///<
                typedef Matrix<Scalar, nofStates, 1> StateVector; ///< A state vector describes the state in which the system is (currently) in
                typedef Matrix<Scalar, nofControls, 1> ControlVector; ///< A state vector describes the state in which the system is (currently) in
                typedef Matrix<Scalar, nofControls, nofStates> FeedbackMatrix; ///< A feedback matrix is used to calculate the optimal control signal, given a current state fo the system, to bring the system to a zero-state
                typedef Matrix<Scalar, nofControls, nofControls> ControlWeightMatrix;

            private:
                Scalar alpha;

            public:
                ControlVector u;
                FeedbackMatrix L;
                StateMatrix A, P, deltaP, Q;
                ControlMatrix B;
                ControlWeightMatrix R;

                LqController() : alpha(1.0) {
                    A.setZero();
                    B.setZero();
                    u.setZero();
                    L.setZero();
                    Q.setIdentity();
                    R.setIdentity();
                    resetRiccati();
                }

                void resetRiccati() {
                    P = Q;
                }

                template<bool isDiscrete>
                void updateModel(const StateMatrix& A_, const ControlMatrix& B_) {
                    A = A_;
                    LOG_EVENT(typeid(Self).name(), 50, "A::::::::::::::\n" << A << "\n:::::::::::::::::::::::::::::::::::::");
                    B = B_;
                    LOG_EVENT(typeid(Self).name(), 50, "B::::::::::::::\n" << B << "\n:::::::::::::::::::::::::::::::::::::");
                    updateControlMatrices(isDiscrete);
                }

                void updateControlMatrices(const bool modelIsDiscrete) {
                    Scalar measure = 1000;
                    Scalar newMeasure;
                    if(modelIsDiscrete) {
                        auto Rinv = R.householderQr();
                        auto BRB = B*Rinv.solve(B.transpose());
                        StateMatrix deltaP;
                        do {
                            deltaP = alpha*(A.transpose()*P + P*A - P*BRB*P + Q).householderQr().solve(P);
                            P += deltaP;
                            newMeasure = deltaP.cwiseAbs2().maxCoeff();
                            //~ std::cout << "(c) Iteration! (" << newMeasure << ")" << std::endl;
                            if(newMeasure > measure) {
                                //~ std::cout << "(c) Wrong way!" << std::endl;
                                break;
                            }
                            measure = newMeasure;
                        } while(measure > P_DELTA_EPSILON);

                        L = Rinv.solve(B.transpose()*P);
                    } else {
                        StateMatrix Pprev;

                        //~ std::cout << "Eig(A): :::::::::::::::::::::::::::" << std::endl << A.eigenvalues().transpose() << std::endl;

                        do {
                            Pprev = P;
                            P = Q + A.transpose()*(P - P*B*(R + B.transpose()*P*B).ldlt().solve(B.transpose()*P))*A;

                            newMeasure = (P-Pprev).cwiseAbs2().maxCoeff();
                            if(newMeasure > measure) {
                                //~ std::cout << "(d) Wrong way!" << std::endl;
                                break;
                            }
                            measure = newMeasure;
                        } while(measure > P_DELTA_EPSILON);

                        L = (R + B.transpose()*P*B).ldlt().solve(B.transpose()*P*A);
                    }
                    LOG_EVENT(typeid(Self).name(), 50, "L::::::::::::::\n" << L << "\n:::::::::::::::::::::::::::::::::::::" << std::endl);
                }

                /*!
                 * \brief   Calculate the control signal optimal for bringing the system to the origin
                 * \param   x   Current state of system
                 * \return  Optimal control signal, given cost matrices of model
                 */
                const ControlVector& control_signal(const StateVector& x) {
                    /// [2] eq. 9.11: u = L*x, r = 0
                    u = -L*x;
                    return u;
                }

                const ControlVector& operator()(const StateVector& x) {
                    return control_signal(x);
                }

                const ControlVector& operator()(const StateVector& x, const StateVector& r) {
                    return control_signal(x-r);
                }
        };
    }
}
/// [2]: Glad & Ljung, Reglerteori (Studentlitteratur, 2003)

#endif
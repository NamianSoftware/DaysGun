// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/PlayerAnimInstance.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/BaseCharacter.h"

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	SetReferences();
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!PlayerRef) return;

	SetEssentialMovementData();
	DetermineLocomotionState();
	TrackLocomotionStates();
}

void UPlayerAnimInstance::NativePostEvaluateAnimation()
{
	Super::NativePostEvaluateAnimation();
	if (!PlayerRef) return;

	UpdateCharacterRotation();
	ResetTransition();
}

void UPlayerAnimInstance::SetReferences()
{
	if (const auto playerRef = Cast<ABaseCharacter>(TryGetPawnOwner()))
	{
		PlayerRef = playerRef;
		CharacterMovementRef = PlayerRef->GetCharacterMovement();
	}
}

void UPlayerAnimInstance::SetEssentialMovementData()
{
	UpdateVelocity();
	UpdatePlayerInput();

	MaxSpeed = CharacterMovementRef->MaxWalkSpeed;
	IsFalling = CharacterMovementRef->IsFalling();
	ActorRotation = PlayerRef->GetActorRotation();

	UpdateInputVectorRotationRate();
	UpdateLean();
}

void UPlayerAnimInstance::DetermineLocomotionState()
{
	PrevLocomotionState = LocomotionState;

	if (IsFalling || StartedJumping)
	{
		LocomotionState = ELocomotionState::ELS_Jump;
		return;
	}

	TimeInLocomotionState += GetDeltaSeconds();
	if (!UKismetMathLibrary::Greater_DoubleDouble(TimeInLocomotionState, MinTimeInLocomotionState)) return;

	DetermineGroundLocomotionState();
}

void UPlayerAnimInstance::TrackLocomotionStates()
{
	TrackLocomotionState(ELocomotionState::ELS_Idle, EntryFlags.IdleFlag,
	                     &UPlayerAnimInstance::OnEntryIdle,
	                     &UPlayerAnimInstance::OnExitIdle,
	                     &UPlayerAnimInstance::WhileTrueIdle,
	                     &UPlayerAnimInstance::WhileFalseIdle);

	TrackLocomotionState(ELocomotionState::ELS_Walk, EntryFlags.WalkFlag,
	                     &UPlayerAnimInstance::OnEntryWalk,
	                     &UPlayerAnimInstance::OnExitWalk,
	                     &UPlayerAnimInstance::WhileTrueWalk,
	                     &UPlayerAnimInstance::WhileFalseWalk);

	TrackLocomotionState(ELocomotionState::ELS_Run, EntryFlags.RunFlag,
	                     &UPlayerAnimInstance::OnEntryRun,
	                     &UPlayerAnimInstance::OnExitRun,
	                     &UPlayerAnimInstance::WhileTrueRun,
	                     &UPlayerAnimInstance::WhileFalseRun);

	TrackLocomotionState(ELocomotionState::ELS_Crouch, EntryFlags.CrouchFlag,
	                     &UPlayerAnimInstance::OnEntryCrouch,
	                     &UPlayerAnimInstance::OnExitCrouch,
	                     &UPlayerAnimInstance::WhileTrueCrouch,
	                     &UPlayerAnimInstance::WhileFalseCrouch);

	TrackLocomotionState(ELocomotionState::ELS_Jump, EntryFlags.JumpFlag,
	                     &UPlayerAnimInstance::OnEntryJump,
	                     &UPlayerAnimInstance::OnExitJump,
	                     &UPlayerAnimInstance::WhileTrueJump,
	                     &UPlayerAnimInstance::WhileFalseJump);
}

void UPlayerAnimInstance::UpdateCharacterRotation()
{
	if (InCycleState())
	{
		CycleRotationBehavior();
	}
	else if (InStartState())
	{
		StartRotationBehavior();
	}
}

void UPlayerAnimInstance::ResetTransition()
{
	PlayStartAnim = false;
	PlayGaitTransitionAnim = false;
}

void UPlayerAnimInstance::UpdateVelocity()
{
	PrevVelocity = Velocity;

	Velocity = CharacterMovementRef->Velocity;
	GroundSpeed = Velocity.Size2D();
}

void UPlayerAnimInstance::UpdatePlayerInput()
{
	InputVectorLastFrame = InputVector;

	InputVector = CharacterMovementRef->GetLastInputVector();
	InputVector = UKismetMathLibrary::ClampVectorSize(InputVector, 0.0, 1.0);
}

void UPlayerAnimInstance::UpdateInputVectorRotationRate()
{
	if (InputVector.IsNearlyZero())
	{
		InputVectorRotationRate = 0.f;
		return;
	}

	const auto NormalizedInputVector = UKismetMathLibrary::Normal(InputVector);
	const auto NormalizedInputVectorLastFrame = UKismetMathLibrary::Normal(InputVectorLastFrame);

	const auto InputVectorRotator = UKismetMathLibrary::MakeRotFromX(NormalizedInputVector);
	const auto InputVectorLastFrameRotator = UKismetMathLibrary::MakeRotFromX(NormalizedInputVectorLastFrame);

	const auto Delta = UKismetMathLibrary::NormalizedDeltaRotator(InputVectorRotator, InputVectorLastFrameRotator);

	const auto DeltaSeconds = GetDeltaSeconds();
	const auto InputVectorRotationRateTarget = Delta.Yaw / DeltaSeconds;

	InputVectorRotationRate = UKismetMathLibrary::FInterpTo(
		InputVectorRotationRate,
		InputVectorRotationRateTarget,
		DeltaSeconds,
		InputVectorRotationRateInterpSpeed
	);
}

void UPlayerAnimInstance::UpdateLean()
{
	const auto WorldDeltaSeconds = GetWorld()->GetDeltaSeconds();

	const auto VelocitySubtraction = UKismetMathLibrary::Subtract_VectorVector(
		FVector(Velocity.X, Velocity.Y, 0), PrevVelocity);
	Acceleration = VelocitySubtraction / WorldDeltaSeconds;

	const bool IsGainingMomentum = UKismetMathLibrary::Greater_DoubleDouble(
		UKismetMathLibrary::DotProduct2D(FVector2D(Acceleration), FVector2D(Velocity)), 0.f);

	const float MaxAcceleration = IsGainingMomentum
		                              ? CharacterMovementRef->GetMaxAcceleration()
		                              : CharacterMovementRef->GetMaxBrakingDeceleration();

	const auto ClampedAcceleration = UKismetMathLibrary::Vector_ClampSizeMax(Acceleration, MaxAcceleration);
	const auto RelativeAccelerationAmount = ActorRotation.UnrotateVector(ClampedAcceleration / MaxAcceleration);

	Lean = UKismetMathLibrary::VInterpTo(
		Lean,
		RelativeAccelerationAmount,
		WorldDeltaSeconds,
		LeanInterpSpeed
	);

	const auto LeanXPower = GetCurveValue(MoveDataLeanXName);
	const auto LeanYPower = GetCurveValue(MoveDataLeanYName);

	LeanX = Lean.X * LeanXPower;
	LeanY = Lean.Y * LeanYPower;
}

void UPlayerAnimInstance::DetermineGroundLocomotionState()
{
	const auto NormalizedVelocity = UKismetMathLibrary::Normal(Velocity);
	const auto NormalizedCurrentAcceleration = UKismetMathLibrary::Normal(
		CharacterMovementRef->GetCurrentAcceleration());

	const auto VelocityAccelerationDotProduct = FVector::DotProduct(NormalizedVelocity, NormalizedCurrentAcceleration);

	if (UKismetMathLibrary::Less_DoubleDouble(VelocityAccelerationDotProduct, -0.5))
	{
		LocomotionState = ELocomotionState::ELS_Idle;
		return;
	}

	if (IsMovementWithinThresholds(RunMinCurrentSpeed, RunMinMaxSpeed, RunMinInputAcceleration))
	{
		LocomotionState = ELocomotionState::ELS_Run;
		return;
	}

	if (IsMovementWithinThresholds(WalkMinCurrentSpeed, WalkMinMaxSpeed, WalkMinInputAcceleration))
	{
		LocomotionState = ELocomotionState::ELS_Walk;
		return;
	}

	LocomotionState = ELocomotionState::ELS_Idle;
}

void UPlayerAnimInstance::UpdateStop()
{
	if (GroundSpeed > RunStopSpeedLimit)
	{
		StopAnim = RunStopAnim;
		AnimStartTime = RunAnimStartTime;
	}
	else
	{
		StopAnim = WalkStopAnim;
		AnimStartTime = WalkAnimStartTime;
	}
}

void UPlayerAnimInstance::UpdateOnWalkEntry()
{
	UpdateEntryVariables();
	UpdateStartAnim(WalkStartAnim,
	                WalkStart90LAnim, WalkStart90LAnimTime,
	                WalkStart180LAnim, WalkStart180LAnimTime,
	                WalkStart90RAnim, WalkStart90RAnimTime,
	                WalkStart180RAnim, WalkStart180RAnimTime,
	                WalkStartFAnim, WalkStartFAnimTime);
}

void UPlayerAnimInstance::UpdateOnRunToWalk()
{
	UpdateTransitionAnim(RunToWalkAnim,
	                     RunToWalkLFAnim,
	                     RunToWalkLFTime,
	                     RunToWalkRFAnim,
	                     RunToWalkRFTime);
}

void UPlayerAnimInstance::UpdateOnRunEntry()
{
	UpdateEntryVariables();
	UpdateStartAnim(RunStartAnim,
	                RunStart90LAnim, RunStart90LAnimTime,
	                RunStart180LAnim, RunStart180LAnimTime,
	                RunStart90RAnim, RunStart90RAnimTime,
	                RunStart180RAnim, RunStart180RAnimTime,
	                RunStartFAnim, RunStartFAnimTime);
}

void UPlayerAnimInstance::UpdateOnWalkToRun()
{
	UpdateTransitionAnim(WalkToRunAnim,
	                     WalkToRunLFAnim,
	                     WalkToRunLFTime,
	                     WalkToRunRFAnim,
	                     WalkToRunRFTime);
}

void UPlayerAnimInstance::UpdateLocomotionValues()
{
	const auto MoveDataSpeed = GetCurveValue(MoveDataSpeedCurveName);
	const auto ClampedMoveDataSpeed = FMath::Clamp(MoveDataSpeed, MoveDataSpeedMinClampValue,
	                                               MoveDataSpeedMaxClampValue);

	PlayRate = UKismetMathLibrary::SafeDivide(GroundSpeed, ClampedMoveDataSpeed);
}

void UPlayerAnimInstance::CycleRotationBehavior()
{
	if (UKismetMathLibrary::NotEqual_RotatorRotator(ActorRotation, TargetRotationSmoothed, 0.1f))
	{
		TargetRotation = ActorRotation;
		TargetRotationSmoothed = ActorRotation;
	}

	CalculateTargetRotationSmoothed();

	const auto NewRotation = FRotator(0.f, TargetRotationSmoothed.Yaw, 0.f);
	PlayerRef->SetActorRotation(NewRotation);
}

void UPlayerAnimInstance::StartRotationBehavior()
{
	const auto TargetRotationBeforeInterp = TargetRotationSmoothed;
	CalculateTargetRotationSmoothed();

	const auto TargetRotationDelta = UKismetMathLibrary::NormalizedDeltaRotator(
		TargetRotationSmoothed, TargetRotationBeforeInterp).Yaw;
	StartAngle += TargetRotationDelta;

	const auto RotationBlendValue = GetCurveValue(MoveDataRotationBlendName);
	const auto DeltaAngle = UKismetMathLibrary::Multiply_DoubleDouble(StartAngle, RotationBlendValue);

	const auto NewRotation = UKismetMathLibrary::ComposeRotators(StartRotation, FRotator(0.f, DeltaAngle, 0.f));
	PlayerRef->SetActorRotation(NewRotation);
}

void UPlayerAnimInstance::UpdateEntryVariables()
{
	StartRotation = ActorRotation;

	const auto MovementVector = GroundSpeed > 15.f ? InputVector : Velocity;
	TargetRotation = UKismetMathLibrary::MakeRotFromX(MovementVector);
	TargetRotationSmoothed = TargetRotation;

	StartAngle = UKismetMathLibrary::NormalizedDeltaRotator(TargetRotation, StartRotation).Yaw;
}

void UPlayerAnimInstance::CalculateTargetRotationSmoothed()
{
	const auto ConstRotationRate = CalculateConstRotationRate();
	const auto WorldDeltaSeconds = GetWorld()->GetDeltaSeconds();

	TargetRotation = UKismetMathLibrary::RInterpTo_Constant(
		TargetRotation,
		UKismetMathLibrary::MakeRotFromX(Velocity),
		WorldDeltaSeconds,
		ConstRotationRate
	);

	TargetRotationSmoothed = UKismetMathLibrary::RInterpTo_Constant(
		TargetRotationSmoothed,
		TargetRotation,
		WorldDeltaSeconds,
		ConstRotationRate
	);
}

bool UPlayerAnimInstance::IsMovementWithinThresholds(float MinCurrentSpeed, float MinMaxSpeed,
                                                     float MinInputAcceleration) const

{
	return UKismetMathLibrary::LessEqual_DoubleDouble(MinCurrentSpeed, GroundSpeed) &&
		UKismetMathLibrary::LessEqual_DoubleDouble(MinMaxSpeed, MaxSpeed) &&
		UKismetMathLibrary::LessEqual_DoubleDouble(MinInputAcceleration, InputVector.Size());
}

void UPlayerAnimInstance::TrackLocomotionState(ELocomotionState TracedState, bool& EnterFlag,
                                               void (UPlayerAnimInstance::*OnEnterCallback)(),
                                               void (UPlayerAnimInstance::*OnExitCallback)(),
                                               void (UPlayerAnimInstance::*WhileTrueCallback)(),
                                               void (UPlayerAnimInstance::*WhileFalseCallback)())
{
	if (LocomotionState == TracedState)
	{
		if (EnterFlag != true)
		{
			EnterFlag = true;
			(this->*OnEnterCallback)();
			TimeInLocomotionState = 0.f;
		}
		else
		{
			(this->*WhileTrueCallback)();
		}
	}
	else
	{
		if (EnterFlag != false)
		{
			EnterFlag = false;
			(this->*OnExitCallback)();
		}
		else
		{
			(this->*WhileFalseCallback)();
		}
	}
}

void UPlayerAnimInstance::UpdateStartAnim(UAnimSequence*& FinishAnim, UAnimSequence* Start90LAnim,
                                          const float Start90LAnimTime, UAnimSequence* Start180LAnim, const float Start180LAnimTime,
                                          UAnimSequence* Start90RAnim,
                                          const float Start90RAnimTime, UAnimSequence* Start180RAnim, const float Start180RAnimTime,
                                          UAnimSequence* StartFAnim,
                                          const float StartFAnimTime)
{
	if (UKismetMathLibrary::InRange_FloatFloat(StartAngle, -135, -45, false, true))
	{
		FinishAnim = Start90LAnim;
		AnimStartTime = Start90LAnimTime;
	}
	else if (UKismetMathLibrary::InRange_FloatFloat(StartAngle, -180, -135, true, true))
	{
		FinishAnim = Start180LAnim;
		AnimStartTime = Start180LAnimTime;
	}
	else if (UKismetMathLibrary::InRange_FloatFloat(StartAngle, 45, 135, true, false))
	{
		FinishAnim = Start90RAnim;
		AnimStartTime = Start90RAnimTime;
	}
	else if (UKismetMathLibrary::InRange_FloatFloat(StartAngle, 135, 180, true, true))
	{
		FinishAnim = Start180RAnim;
		AnimStartTime = Start180RAnimTime;
	}
	else
	{
		FinishAnim = StartFAnim;
		AnimStartTime = StartFAnimTime;
	}
}

void UPlayerAnimInstance::UpdateTransitionAnim(UAnimSequence*& FinishAnim, UAnimSequence* TransitionLFAnim,
                                               const float AnimLFStartTime, UAnimSequence* TransitionRFAnim,
                                               const float AnimRFStartTime)
{
	const auto FootPhase = GetCurveValue(MoveDataFootPhaseCurveName);
	if (UKismetMathLibrary::GreaterEqual_DoubleDouble(FootPhase, MoveDataLeftFootPhaseLimit))
	{
		FinishAnim = TransitionLFAnim;
		AnimStartTime = AnimLFStartTime;
	}
	else
	{
		FinishAnim = TransitionRFAnim;
		AnimStartTime = AnimRFStartTime;
	}
}

float UPlayerAnimInstance::CalculateConstRotationRate() const
{
	return UKismetMathLibrary::MapRangeClamped(
		UKismetMathLibrary::Abs(InputVectorRotationRate),
		0.f, 200.f,
		500.f, 2000.f);
}

#pragma region IdleCallbacks
void UPlayerAnimInstance::OnEntryIdle()
{
	UpdateStop();
}

void UPlayerAnimInstance::OnExitIdle()
{
}

void UPlayerAnimInstance::WhileTrueIdle()
{
}

void UPlayerAnimInstance::WhileFalseIdle()
{
}
#pragma endregion

#pragma region WalkCallbacks
void UPlayerAnimInstance::OnEntryWalk()
{
	if (PrevLocomotionState == ELocomotionState::ELS_Run)
	{
		if (UKismetMathLibrary::Less_DoubleDouble(GroundSpeed, MaxSpeedForPlayingStartAnim))
		{
			PlayStartAnim = true;
			UpdateOnWalkEntry();
		}
		else if (UKismetMathLibrary::Less_DoubleDouble(MinTimeGaitTransitionAnim, TimeInLocomotionState))
		{
			PlayGaitTransitionAnim = true;
			UpdateOnRunToWalk();
		}
	}
	else if (PrevLocomotionState == ELocomotionState::ELS_Idle)
	{
		PlayStartAnim = true;
		UpdateOnWalkEntry();
	}
}

void UPlayerAnimInstance::OnExitWalk()
{
}

void UPlayerAnimInstance::WhileTrueWalk()
{
	UpdateLocomotionValues();
}

void UPlayerAnimInstance::WhileFalseWalk()
{
}
#pragma endregion

#pragma region RunCallbacks
void UPlayerAnimInstance::OnEntryRun()
{
	if (PrevLocomotionState == ELocomotionState::ELS_Walk)
	{
		if (StateMachineIsWalkStartState())
		{
			PlayStartAnim = true;
			UpdateOnRunEntry();
		}
		else if (UKismetMathLibrary::Less_DoubleDouble(MinTimeGaitTransitionAnim, TimeInLocomotionState))
		{
			PlayGaitTransitionAnim = true;
			UpdateOnWalkToRun();
		}
	}
	else if (PrevLocomotionState == ELocomotionState::ELS_Idle)
	{
		PlayStartAnim = true;
		UpdateOnRunEntry();
	}
}

void UPlayerAnimInstance::OnExitRun()
{
}

void UPlayerAnimInstance::WhileTrueRun()
{
	UpdateLocomotionValues();
}

void UPlayerAnimInstance::WhileFalseRun()
{
}
#pragma endregion

#pragma region CrouchCallbacks
void UPlayerAnimInstance::OnEntryCrouch()
{
}

void UPlayerAnimInstance::OnExitCrouch()
{
}

void UPlayerAnimInstance::WhileTrueCrouch()
{
	UpdateLocomotionValues();
}

void UPlayerAnimInstance::WhileFalseCrouch()
{
}
#pragma endregion

#pragma region JumpCallbacks
void UPlayerAnimInstance::OnEntryJump()
{
}

void UPlayerAnimInstance::OnExitJump()
{
}

void UPlayerAnimInstance::WhileTrueJump()
{
}

void UPlayerAnimInstance::WhileFalseJump()
{
}
#pragma endregion

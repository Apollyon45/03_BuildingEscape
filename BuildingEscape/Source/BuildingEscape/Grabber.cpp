// Copyright Wolfpack Games 2018

#include "Grabber.h"
#include "Engine/World.h"

#define OUT

// Sets default values for this component's properties
UGrabber::UGrabber()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UGrabber::BeginPlay()
{
	Super::BeginPlay();
	FindPhysicsHandleComponent();
	SetupInputComponent();
}

/// Look for attached physics handle
void UGrabber::FindPhysicsHandleComponent()
{
	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	if (PhysicsHandle == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s is missing physics handle component"), *GetOwner()->GetName());
	}
}
/// Look for attached input component (only appears at runtime)
void UGrabber::SetupInputComponent()
{
	InputComponent = GetOwner()->FindComponentByClass<UInputComponent>();
	if (InputComponent)
	{
		InputComponent->BindAction("Grab", IE_Pressed, this, &UGrabber::Grab);
		InputComponent->BindAction("Grab", IE_Released, this, &UGrabber::Release);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s is missing its Input Component."), *GetOwner()->GetName());
	}
}

// Called every frame
void UGrabber::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PhysicsHandle)
	{
		return;
	}
	// if the physics handle is attached
	if (PhysicsHandle->GrabbedComponent)
	{
		// move object that we're holding
		PhysicsHandle->SetTargetLocation(GetLineTracePoints().v2);
	}
}

void UGrabber::Grab()
{
	/// LINE TRACE and see if we reach any actors with physics body collision channel set
	auto HitResult = GetFirstPhysicsBodyInReach();
	auto ComponentToGrab = HitResult.GetComponent(); // gets the mesh in our case
	auto ActorHit = HitResult.GetActor();

	/// If we hit something then attach a physics handle
	if (ActorHit)
	{
		if (!PhysicsHandle)
		{
			return;
		}
		//PhysicsHandle->GrabComponentAtLocation(ComponentToGrab, NAME_None, ComponentToGrab->GetOwner()->GetActorLocation());
		PhysicsHandle->GrabComponentAtLocationWithRotation(ComponentToGrab, NAME_None, ComponentToGrab->GetOwner()->GetActorLocation(), ComponentToGrab->GetOwner()->GetActorRotation());
	}
	
}

void UGrabber::Release()
{
	if (!PhysicsHandle)
	{
		return;
	}
	PhysicsHandle->ReleaseComponent();
}

FHitResult UGrabber::GetFirstPhysicsBodyInReach() const
{
	// Line trace out to reach distance
	FHitResult HitResult;

	FCollisionQueryParams TraceParameters(FName(TEXT(" ")), false, GetOwner());

	FTwoVectors TracePoints = GetLineTracePoints();

	GetWorld()->LineTraceSingleByObjectType(OUT HitResult, TracePoints.v1, TracePoints.v2, FCollisionObjectQueryParams(ECollisionChannel::ECC_PhysicsBody), TraceParameters);

	return HitResult;
}

FTwoVectors UGrabber::GetLineTracePoints() const
{
	FVector StartLocation;
	FRotator PlayerViewPointRotation;
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(OUT StartLocation, OUT PlayerViewPointRotation);

	FVector EndLocation = StartLocation + PlayerViewPointRotation.Vector() * Reach;

	return FTwoVectors(StartLocation, EndLocation);
}

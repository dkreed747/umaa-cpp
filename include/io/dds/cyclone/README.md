# Cyclone specific information and examples


## How to set a discriminator and instantiate a union subtype
  gvCommand.direction().DirectionRequirementVariantTypeSubtypes().DirectionTrueNorthRequirementVariantVariant(UMAA::Common::Orientation::DirectionTrueNorthRequirementVariantType());
  gvCommand.direction().DirectionRequirementVariantTypeSubtypes().DirectionTrueNorthRequirementVariantVariant().direction().direction(expDirection);
  // Setting the Discriminator using _d is redundant in this case, but is here as an example.
  //gvCommand.direction().DirectionRequirementVariantTypeSubtypes()._d(UMAA::Common::Orientation::DirectionRequirementVariantTypeEnum::DIRECTIONTRUENORTHREQUIREMENTVARIANT_D);

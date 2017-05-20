# Box2D wrapped worlds fork

This fork implements two major feature that I require for my open world game:
 - Spaces (work in progress) - extends collision filtering allowing multiple independent layers of bodies to coexist inside one world.
 - Teleport joint - glues two bodies using defined offset.

**Intended use**
 - Split very large world to chunks with acceptable precision
 - When some body is crossing borders of this chunks, create copies so each space have one
 - Link cloned bodies from different spaces using teleport joint (and remember to divide mass to account for multiple copies of the same object)

## Demos

Simple demo (no dynamic space switching) is available named Wrapped Worlds

## License

Box2D is developed by Erin Catto, and has the [zlib license](http://en.wikipedia.org/wiki/Zlib_License).
My additions are using same license.

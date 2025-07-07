# Documentation Metadata Guidelines

---
**Document Type**: Guide  
**Last Updated**: 2025-01-06  
**Status**: Current  
**Version**: 1.0  
**Component(s)**: All  
**Tags**: documentation, metadata, guidelines, standards  
---

## Purpose

This document defines the metadata header standard for all documentation in the LPZRobots project. Consistent metadata helps with document management, versioning, and navigation.

## Metadata Header Format

All markdown documentation files should include the following metadata header immediately after the title:

```markdown
# Document Title

---
**Document Type**: [Type]  
**Last Updated**: YYYY-MM-DD  
**Status**: [Status]  
**Version**: X.Y  
**Author(s)**: [Optional]  
**Component(s)**: [Component]  
**Tags**: [tag1, tag2, tag3]  
---
```

## Field Definitions

### Document Type
Choose one of the following:
- **Guide** - How-to guides and tutorials
- **Reference** - API documentation and technical references
- **Status** - Project status reports and summaries
- **Architecture** - System design and architecture documents
- **Analysis** - Code analysis, reports, and studies
- **Tutorial** - Step-by-step learning materials
- **Documentation** - General documentation (if none of the above fit)

### Last Updated
- Format: YYYY-MM-DD (e.g., 2025-01-06)
- Update this whenever the document content changes significantly

### Status
- **Current** - Up-to-date and accurate
- **Archived** - Historical document, kept for reference
- **Draft** - Work in progress
- **Under Review** - Awaiting review or approval
- **Deprecated** - Superseded by newer documentation

### Version
- Use semantic versioning (Major.Minor)
- 1.0 for initial release
- Increment minor version for updates
- Increment major version for significant rewrites

### Author(s) (Optional)
- Include for documents with specific attribution needs
- Default: "LPZRobots Team"

### Component(s)
- **selforg** - Core controller library
- **ode_robots** - Robot simulation framework
- **ga_tools** - Genetic algorithm tools
- **build-system** - Build and CMake documentation
- **testing** - Testing infrastructure
- **all** - Cross-cutting documentation

### Tags
Comma-separated list of relevant keywords:
- Common tags: modernization, cmake, testing, algorithms, performance, architecture
- Component-specific tags: homeokinetic, controller, robot, simulation
- Status tags: complete, in-progress, planned

## Examples

### Guide Example
```markdown
# Quick Start Guide

---
**Document Type**: Guide  
**Last Updated**: 2025-01-06  
**Status**: Current  
**Version**: 2.0  
**Component(s)**: All  
**Tags**: quickstart, installation, tutorial, getting-started  
---
```

### Status Report Example
```markdown
# Migration Status Report

---
**Document Type**: Status  
**Last Updated**: 2025-01-06  
**Status**: Current  
**Version**: 1.5  
**Author(s)**: Migration Team  
**Component(s)**: All  
**Tags**: migration, C++17, Qt6, progress, status  
---
```

## Implementation

### Manual Addition
Add the metadata header when creating new documentation files.

### Automated Addition
Use the provided script to add metadata to existing files:
```bash
./scripts/add_metadata_headers.sh docs/
```

The script will:
- Skip files that already have metadata
- Automatically determine document type based on filename
- Set appropriate component based on file path
- Use current date for "Last Updated"

### Validation
When reviewing documentation:
1. Check that metadata is present and complete
2. Verify the document type is appropriate
3. Ensure the status reflects current state
4. Update version and date when making changes

## Benefits

1. **Discoverability** - Easy to find documents by type, component, or tags
2. **Maintenance** - Clear indication of document status and currency
3. **Navigation** - Automated index generation possible
4. **Version Control** - Track document evolution
5. **Quality Assurance** - Identify outdated or draft documents

## Future Enhancements

- Automated metadata validation in CI/CD
- Document index generation from metadata
- Stale document detection based on last updated date
- Tag-based documentation search
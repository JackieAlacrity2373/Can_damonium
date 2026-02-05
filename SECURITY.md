# Security Policy

## Reporting a Vulnerability

We take security vulnerabilities in Can_damonium seriously. If you discover a security vulnerability, please **do not** open a public GitHub issue. Instead, please report it responsibly.

### How to Report

1. **Email**: Send a detailed report to the maintainer at your earliest convenience
2. **Include in your report**:
   - Description of the vulnerability
   - Steps to reproduce (if applicable)
   - Potential impact
   - Suggested fix (if you have one)

### Response Timeline

- **Initial acknowledgment**: Within 48 hours
- **Assessment**: Within 1 week
- **Fix/Patch**: Target within 2-4 weeks depending on severity
- **Disclosure**: After a fix is released or 90 days from report, whichever comes first

## Supported Versions

| Version | Supported          |
|---------|-------------------|
| Latest  | ✅ Fully supported |
| Older   | 🔶 Best effort     |

We recommend always using the latest version to ensure you have the latest security fixes.

## Security Best Practices

### For Users

- **Keep updated**: Always use the latest version of Can_damonium
- **Audio files**: Only load IR (impulse response) files from trusted sources
- **System security**: Run your DAW/host in a secure environment
- **Permissions**: Install the plugin with appropriate user privileges

### For Developers Contributing to This Project

- **Dependencies**: Keep JUCE and all dependencies up to date
- **Code review**: All contributions go through review before merging
- **Testing**: Enable compiler warnings and address them
- **Memory safety**: Be cautious with buffer operations and file handling
- **Input validation**: Validate all user input, especially file paths and audio data

## Security Considerations

### Audio Processing
- The convolution engine processes audio data safely with bounds checking
- IR files are validated before loading
- Memory allocation is tracked and managed

### File Operations
- File paths are validated
- Resource loading uses secure file APIs
- Paths are relative to known safe directories

### Plugin Interface
- VST3 and AU interfaces are implemented per official specifications
- Parameter ranges are enforced
- No arbitrary code execution from parameter values

## Known Limitations

- This is an audio plugin, not a security-critical application
- Security review has been performed on core audio processing paths
- Some dependencies (JUCE framework) are maintained by third parties

## Vulnerability Disclosure Timeline

We follow a responsible disclosure process:

1. Report received
2. Vulnerability assessed and reproduced
3. Fix developed and tested
4. Security advisory drafted (if applicable)
5. Coordinated release with disclosure

## Contact

For security concerns, please contact the project maintainer through GitHub profile.

## Security Updates

Check the [releases page](https://github.com/JackieAlacrity2373/Can_damonium/releases) for security-related updates and patches.

## Disclaimer

This security policy outlines our commitment to addressing security issues. However, no software is completely free of vulnerabilities. Use Can_damonium in a manner consistent with its intended purpose and your security requirements.

---

Last Updated: February 4, 2026

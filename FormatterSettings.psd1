@{
  IncludeRules = @(
    'PSUseConsistentWhitespace'
    'PSUseConsistentIndentation'
    'PSPlaceCloseBrace'
    'PSPlaceOpenBrace'
    'PSAlignAssignmentStatement'
  )

  Rules        = @{
    PSUseConsistentIndentation = @{
      Enable              = $true
      Kind                = 'space'
      IndentationSize     = 2
      PipelineIndentation = 'IncreaseIndentationForFirstPipeline'
    }
    PSUseConsistentWhitespace  = @{
      Enable          = $true
      CheckInnerBrace = $true
      CheckOpenBrace  = $true
      CheckOpenParen  = $true
      CheckOperator   = $true
      CheckPipe       = $true
      CheckSeparator  = $true
    }

    PSPlaceOpenBrace           = @{
      Enable             = $true
      OnSameLine         = $true
      NewLineAfter       = $true
      IgnoreOneLineBlock = $true
    }

    PSPlaceCloseBrace          = @{
      Enable             = $true
      NewLineAfter       = $false
      IgnoreOneLineBlock = $true
    }

    PSAlignAssignmentStatement = @{
      Enable         = $true
      CheckHashtable = $true
    }
  }
}


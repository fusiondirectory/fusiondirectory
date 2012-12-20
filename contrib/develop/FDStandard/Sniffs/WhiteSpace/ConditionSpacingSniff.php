<?php
/**
 * FDStandard_Sniffs_WhiteSpace_AssignmentSpacingSniff.
 *
 * Checks that assignment blocks have aligned equals signs
 *
 * Modified version of PHP_CodeSniffer Generic_Sniffs_Function_FunctionCallArgumentSpacingSniff
 * by Greg Sherwood <gsherwood@squiz.net> and Marc McIntyre <mmcintyre@squiz.net>
 *
 * @author    Côme Bernigaud <come.bernigaud@laposte.net>
 * @license   http://matrix.squiz.net/developer/tools/php_cs/licence BSD Licence
 */

class FDStandard_Sniffs_WhiteSpace_ConditionSpacingSniff implements PHP_CodeSniffer_Sniff
{
    /**
     * Returns an array of tokens this test wants to listen for.
     *
     * @return array
     */
    public function register()
    {
        return array( T_WHILE, T_FOR, T_FOREACH, T_IF, T_ELSE, T_ELSEIF,
                      T_DO, T_TRY, T_CATCH, T_SWITCH);

    }//end register()


    /**
     * Processes this test, when one of its tokens is encountered.
     *
     * @param PHP_CodeSniffer_File $phpcsFile The file being scanned.
     * @param int                  $stackPtr  The position of the current token in the
     *                                        stack passed in $tokens.
     *
     * @return void
     */
    public function process(PHP_CodeSniffer_File $phpcsFile, $stackPtr)
    {
        $tokens = $phpcsFile->getTokens();

        //~ $blockClosers = array(T_CLOSE_CURLY_BRACKET, T_CLOSE_SQUARE_BRACKET, T_CLOSE_PARENTHESIS);

        if ($prevBracket = $phpcsFile->findPrevious(array(T_CLOSE_CURLY_BRACKET),$stackPtr-1)) {
          if ($tokens[$prevBracket]['line'] == $tokens[$stackPtr]['line']) {
              if ($tokens[$stackPtr-1]['code'] !== T_WHITESPACE) {
                  //Check that there is a space between this bracket and the condition token
                  $error = 'Condition token "'.$tokens[$stackPtr]['content'].'" should be preceded by a space';
                  $phpcsFile->addError($error, $stackPtr, 'ConditionBeforeSpace');
              }
          } elseif (in_array($tokens[$stackPtr]['code'],array(T_ELSE, T_ELSEIF, T_CATCH))) {
              //The brace should be there
              $error = 'The brace before "'.$tokens[$stackPtr]['content'].'" should be on the same line';
              $phpcsFile->addError($error, $prevBracket, 'ConditionBeforeBracket');
          }
        }

        if ($nextBracket = $phpcsFile->findNext(array(T_OPEN_CURLY_BRACKET),$stackPtr+1)) {
          $closingParenthesis = $phpcsFile->findPrevious(array(T_CLOSE_PARENTHESIS),$nextBracket);
          $needsParenthesis   = in_array($tokens[$stackPtr]['code'],array(T_ELSEIF, T_IF));
          if (($needsParenthesis && ($tokens[$nextBracket]['line'] != $tokens[$closingParenthesis]['line'])) ||
              (!$needsParenthesis && ($tokens[$nextBracket]['line'] != $tokens[$stackPtr]['line']))) {
              $error = 'The brace after "'.$tokens[$stackPtr]['content'].'" should be on the same line';
              $phpcsFile->addError($error, $nextBracket, 'ConditionAfterBracketLine');
          } elseif ($tokens[$nextBracket-1]['code'] !== T_WHITESPACE) {
              $error = 'The "'.$tokens[$nextBracket]['content'].'" after "'.$tokens[$stackPtr]['content'].'" should be preceded by a space';
              $phpcsFile->addError($error, $stackPtr, 'ConditionAfterBracketSpace');
              return;
          }
        }

        //Check that there is a space after the condition token
        if ($tokens[$stackPtr+1]['code'] !== T_WHITESPACE) {
            $error = 'Condition token "'.$tokens[$stackPtr]['content'].'" should be followed by a space';
            $phpcsFile->addError($error, $stackPtr, 'ConditionAfterSpace');
        }

    }//end process()


}//end class

?>
